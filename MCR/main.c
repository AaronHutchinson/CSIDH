#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <time.h>

#include "u512.h"
#include "fp.h"
#include "mont.h"
#include "csidh.h"
#include "cycle.h"

void u512_print(u512 const *x) {
	for (size_t i = 63; i < 64; --i)
		printf("%02hhx", i[(unsigned char *) x->c]);
}
void fp_print(fp const *x) {
	u512 y;
	fp_dec(&y, x);
	u512_print(&y);
}
int main() {
	clock_t t0, t1;
	//int8_t exponent_bounds[num_primes] = { 9, 17, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 18, 19, 19, 18, 18, 18, 16, 16, 15, 14, 14, 12, 13, 11, 12, 11, 11, 10, 10, 10, 9, 9, 9, 9, 8, 8, 8, 7, 8, 7, 7, 7, 7, 6, 7, 6, 6, 7, 6, 7, 7, 6, 6, 6, 6, 6, 6, 6, 6, 6, 5, 5, 5, 5, 5, 5, 6, 4};
	srand(time(NULL));

	private_key priv_alice, priv_bob;
	public_key pub_alice, pub_bob;
	public_key shared_alice, shared_bob;

	int8_t exponent_bounds[num_primes];
	assign_bounds(exponent_bounds);
	
	// calculate inverses for "elligatoring"
	// compute inverse of u^2 - 1 : from u in {2,..,10}
	for (int i = 2; i <= 10; i++) {
		fp_set(&invs_[i - 2], i);
		fp_sq1(&invs_[i - 2]);
		fp_sub2(&invs_[i - 2], &fp_1);
		fp_inv(&invs_[i - 2]);
	}

	// Choose private key for Alice
	t0 = clock();
	csidh_private(&priv_alice, exponent_bounds);
	t1 = clock();

	printf("Alice's private key   (%7.3lf ms):\n  ", 1000. * (t1 - t0) / CLOCKS_PER_SEC);
	for (size_t i = 0; i < sizeof(priv_alice); ++i)
		printf("%01x", i[(int8_t *) &priv_alice]);
	printf("\n\n");

	// Choose private key for Bob
	t0 = clock();
	csidh_private(&priv_bob, exponent_bounds);
	t1 = clock();

	printf("Bob's private key     (%7.3lf ms):\n  ", 1000. * (t1 - t0) / CLOCKS_PER_SEC);
	for (size_t i = 0; i < sizeof(priv_alice); ++i)
		printf("%01x", i[(int8_t *) &priv_bob]);
	printf("\n\n");
	assert(csidh(&pub_alice, &base, &priv_alice, exponent_bounds));

	// Compute public key for Alice
	t0 = clock();
	assert(csidh(&pub_alice, &base, &priv_alice, exponent_bounds));
	t1 = clock();

	printf("Alice's public key (including validation) (%7.3lf ms):\n  ", 1000. * (t1 - t0) / CLOCKS_PER_SEC);
	for (size_t i = 0; i < sizeof(pub_alice); ++i)
		printf("%02hhx", i[(uint8_t *) &pub_alice]);
	printf("\n\n");

	// Compute public key for Bob
	t0 = clock();
	assert(csidh(&pub_bob, &base, &priv_bob, exponent_bounds));
	t1 = clock();

	printf("Bob's public key (including validation) (%7.3lf ms):\n  ", 1000. * (t1 - t0) / CLOCKS_PER_SEC);
	for (size_t i = 0; i < sizeof(pub_bob); ++i)
		printf("%02hhx", i[(uint8_t *) &pub_bob]);
	printf("\n\n");

	// Compute action of Alice's private key on Bob's public key
	t0 = clock();
	assert(csidh(&shared_alice, &pub_bob, &priv_alice, exponent_bounds));
	t1 = clock();

	printf("Alice's shared secret (including validation) (%7.3lf ms):\n  ", 1000. * (t1 - t0) / CLOCKS_PER_SEC);
	for (size_t i = 0; i < sizeof(shared_alice); ++i)
		printf("%02hhx", i[(uint8_t *) &shared_alice]);
	printf("\n\n");

	// Compute action of Bob's private key on Alice's public key
	t0 = clock();
	assert(csidh(&shared_bob, &pub_alice, &priv_bob, exponent_bounds));
	t1 = clock();

	printf("Bob's shared secret (including validation) (%7.3lf ms):\n  ", 1000. * (t1 - t0) / CLOCKS_PER_SEC);
	for (size_t i = 0; i < sizeof(shared_bob); ++i)
		printf("%02hhx", i[(uint8_t *) &shared_bob]);
	printf("\n\n");

	if (memcmp(&shared_alice, &shared_bob, sizeof(public_key)))
		printf("\x1b[31mNOT EQUAL! :(\x1b[0m\n");
	else
		printf("\x1b[32mequal :)\x1b[0m\n");
	printf("\n");


}
