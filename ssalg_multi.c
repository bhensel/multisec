#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include "sha1.h"
#include "ssalg_multi.h"
#include "fieldpoly/fieldpoly.h"
#include "fieldpoly/primeField.h"

static const struct ssalg_ops ssalg_multi_ops;



//init for multi.
//should have a primeField/some other field attached to the struct here.
int ssalg_multi_init(struct ssalg_multi *this, int n, int k)
{
	assert(n > 0);

	if (ssalg_init_internal(&this->super, &ssalg_multi_ops))
		return 1;

	this->n = n;
	this->k = k;

	return 0;
}

//getter for n, the number of channels. Required by ssalg.h
static int ssalg_multi_get_n(const struct ssalg *super)
{
	const struct ssalg_multi *this = (const struct ssalg_multi *) super;

	return this->n;
}

//getter for k, the threshold.  Required by ssalg.h
static int ssalg_multi_get_k(const struct ssalg *super)
{
	const struct ssalg_multi *this = (const struct ssalg_multi *) super;

	return this->k;
}

//helper method, returns n unique random elements of a field.
//bound provides a lower limit for the number.
static void unique_rand2(gf257_element_t** rands, int n, int bound)
{
	//counter for the loop.
	int i = 0;
	//for the random elements.
	gf257_element_t* result = malloc(sizeof(gf257_element_t));
	gf257_init(result);

	while(i < n)
	{
		gf257_randelement((element_t*) result);
		if(result->contents > bound)
		{
			int j;
			int present = 0;
			for (j = 0; j < i; j++)
			{
				if(rands[j]->contents == result->contents)
				{
					present = 1;
				}
			}

			if(!present)
			{
				rands[i] = malloc(sizeof(gf257_element_t));
				gf257_init(rands[i]);
				rands[i]->contents = result->contents;
				i++;
			}
		}
	}
}

static void unique_rand(gf257_element_t *rands, int n, int lbound)
{
	int i = 0;
	while (i < n) {
		// Fill in next index with random element
		gf257_init(&rands[i]);
		gf257_randelement(&rands[i].super);

		// Ensure element is greater than lbound
		if (rands[i].contents <= lbound)
			continue;

		// Ensure element is not already chosen
		for (int j = 0; j < i; j++)
			if (rands[j].contents == rands[i].contents)
				continue;

		// It's good, move on
		i++;
	}
}

//convert an element into a buffer
//if contents is 255 make buf[0] 255 and buf[1] 0.
//if contents is 256 make buf[0] 255 and buf[1] 1.
static size_t buf_convert(gf257_element_t* el, uint8_t *buf)
{
	int numBytes = 1;
	buf[0] = el->contents;
	if(el->contents > 254)
	{
		numBytes = 2;
		buf[0] = 255;
		buf[1] = el->contents-255;
	}

	return numBytes;
}

//buf should always have a length of at least two.
static void bytes_convert(gf257_element_t* el, uint8_t *buf)
{
	if(buf[0] == 255)
	{
		if(buf[1] == 0)
		{
			el->contents = 255;
		}
		else
		{
			el->contents = 256;
		}
	}
	else{
		el->contents = buf[0];
	}

}

static void one_round(const struct ssalg_multi *this, const uint8_t *inbuf, size_t len, uint8_t *const *out, size_t *outlen)
{

	element_t** xCords = calloc((this->n)+len, sizeof(xCords[0]));

	element_t** yCords = calloc((this->n)+len, sizeof(yCords[0]));

	gf257_element_t* rands = calloc((this->n)*2, sizeof(gf257_element_t));
	unique_rand(rands, (this->n)*2, len);

	int counter;
	for(counter = 0; counter<len; counter++)
	{
		//all these temps go unfreed.  Fix eventually
		gf257_element_t* temp = malloc(sizeof(gf257_element_t));
		gf257_init(temp);
		temp->contents = counter;
		xCords[counter] = &temp->super;

		temp = malloc(sizeof(gf257_element_t));
		gf257_init(temp);
		temp->contents = inbuf[counter];
		yCords[counter] = &temp->super;
	}



	gf257_element_t R;
	gf257_init(&R);
	gf257_randelement(&R.super);

	int i;
	for (i = 0; i < this->n; i++)
	{
		//initialize hash
		SHA1_CTX hash;
		SHA1Init(&hash);

		uint8_t Rbuf[2]; 

		//make R a buffer
		int numBytes;
		numBytes = buf_convert(&R, Rbuf);

		//add to hash
		SHA1Update(&hash, Rbuf, numBytes);

		//make Si a buffer
		gf257_element_t S = rands[i];
		uint8_t Sbuf[2];

		numBytes = buf_convert(&S, Sbuf);

		//add to hash
		SHA1Update(&hash, Sbuf, numBytes);

		//hash
		uint8_t digest[20];
		SHA1Final(&hash, digest);

		gf257_element_t hashed;
		gf257_init(&hashed);

		//turn digest into an element.
		bytes_convert(&hashed, digest);

		//load coords.
		gf257_element_t* temp = malloc(sizeof(gf257_element_t));
		gf257_init(temp);
		temp->contents = rands[i+(this->n)].contents; //Ui
		xCords[counter+i] = &temp->super;

		gf257_init(temp);
		temp->contents = hashed.contents;
		yCords[counter+i] = &temp->super;

	}



	poly_t* polynomial;

	polynomial = interpolate(xCords, yCords, counter+i);
	//hi



	//load the published values into out. 
	int j, k; 
	memcpy(out[0], &R.contents, sizeof(R.contents));
	outlen[0] += sizeof(R.contents);

	for(k = 0; k < (this->n); k++)
	{
		//load s and u
		memcpy(out[k] + outlen[k], &rands[k].contents, sizeof(rands[k].contents));
		outlen[k] += sizeof(rands[k].contents);
		memcpy(out[k] + outlen[k], &rands[k+this->n].contents, sizeof(rands[k+this->n].contents));
		outlen[k] += sizeof(rands[k+this->n].contents);
	}

	gf257_element_t d;
	gf257_init(&d);
	d.contents = 1 + this->n;

	for (j = 0; j < (len); j++)
	{
		int ch = (j+1)%this->n;
		//load evaluated points
		eval_poly(polynomial, (element_t*) &d);
		//starting with channel 1 
		memcpy(out[ch] + outlen[ch], &d.contents, sizeof(d.contents));
		outlen[ch] += sizeof(d.contents);

	}

	free(polynomial);
	free(rands);
	free(xCords);
	free(yCords);

}

//Required by ssalg.h.  Currently in dummy form.
//ss_packets is an array of packets, with one packet per channel (n)
static int ssalg_multi_split(struct ssalg *super, const uint8_t *buf, size_t len, struct ss_packet **shares)
{

	const struct ssalg_multi *this = (const struct ssalg_multi *) super;

	size_t length;
	int offset;

	//array of outs
	uint8_t *out[this->n];
	int i;
	//point out to shares
	for(i = 0; i < this->n; i++)
	{
		out[i] = shares[i]->data;
	}

	//size of each out
	size_t outlen[this->n];
	for(i = 0; i< this->n; i++){
		shares[i]->len = 0;
	}

	for(offset = 0; offset < len; offset+= 200)
	{
		for(i = 0; i< this->n; i++)
			outlen[i] = 0;

		length = len - offset < 200? len-offset: 200;
		one_round(this, buf + offset, length, out, outlen);

		//move out[i] forward for the next pass
		for(i = 0; i < this->n; i++)
		{
			out[i] += outlen[i];
			shares[i]->len += outlen[i];
		}

	}

	for(i = 0; i<this->n; i++)
		shares[i]->len = outlen[i];

	return 0;
}




//Required by ssalg.h.  Currently in dummy form.
static ssize_t ssalg_multi_recombine(struct ssalg *super, uint8_t *buf, size_t len, struct ss_packet **shares)
{
	return 0;
}

/*
static void one_recombine(struct ssalg *super, )
{

}
*/


//Required by ssalg.h, based on ssalg_xor.
static const struct ssalg_ops ssalg_multi_ops = {
	.get_n = ssalg_multi_get_n,
	.get_k = ssalg_multi_get_k,
	.split = ssalg_multi_split,
	.recombine = ssalg_multi_recombine,
	.destroy = NULL, //This probably should be defined eventually (before testing)
};

