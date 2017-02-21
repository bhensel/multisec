#ifndef SSALG_H_
#define SSALG_H_

#include <stdint.h>
#include <sys/types.h>

#include "ss.h"

struct ssalg;

/**
 * struct ssalg_ops - generic interface for a secret-sharing algorithm
 * @get_n: returns n, the number of shares generated from a single message
 * @get_k: returns k, the number of shares required to reconstruct a message
 * @split: generates n shares for the given message, returning 0 for success and
 *      nonzero for error
 * @recombine: recombines k shares to yield the original message, returning the
 *      length of the message, or -1 for error
 * @destroy: frees any resources that were allocated by the given algorithm
 *      implementation during init
 */
struct ssalg_ops {
	int (*get_n)(const struct ssalg *this);
	int (*get_k)(const struct ssalg *this);

	int (*split)(struct ssalg *this, const uint8_t *buf, size_t len,
			struct ss_packet **shares);
	ssize_t (*recombine)(struct ssalg *this, uint8_t *buf, size_t len,
			struct ss_packet **shares);

	void (*destroy)(struct ssalg *this);
};

struct ssalg {
	const struct ssalg_ops *ops;
};

int ssalg_init_internal(struct ssalg *this, const struct ssalg_ops *ops);
void ssalg_destroy_internal(struct ssalg *this);

int ssalg_get_n(const struct ssalg *this);
int ssalg_get_k(const struct ssalg *this);

int ssalg_split(struct ssalg *this, const uint8_t *buf, size_t len,
		struct ss_packet **shares);
ssize_t ssalg_recombine(struct ssalg *this, uint8_t *buf, size_t len,
		struct ss_packet **shares);

void ssalg_destroy(struct ssalg *this);

#endif