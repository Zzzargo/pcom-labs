#ifndef DH_H
#define DH_H

#include <stdint.h>

// chosen arbitrarily
#define PUBLIC_PRIME 5006909
#define PUBLIC_ROOT  10001

/* Computes g^a mod p */
uint32_t mod_pow(uint32_t g, uint32_t a, uint32_t p);
/* Generates a secret, DH secret, based on the public prime number p. */
uint32_t generate_secret(uint32_t p);
/* Takes a shared secret and generates an encryption key. The difference between
 * the two is as follow. Generate_secret returns a for alice. They
 * compute g^(ab) mod p. This is the shared secret. But you now want to
 * have a key that we derive from this secret. We need to do this because
 * the shared secret may have different structure or size. */
uint32_t *derive_key(uint32_t shared_secret);

#endif /* DH_H */
