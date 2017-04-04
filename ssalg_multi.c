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
static size_t bytes_convert(gf257_element_t* el, uint8_t *buf)
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
		return 2;
	}
	else{
		el->contents = buf[0];
		return 1;

	}

}

//simplistic selection sort.
static void sort(gf257_element_t* rands, int n)
{
	int q, w, temp, min, ind;
	for(q = 0; q < n; q++)
	{
		min = rands[q].contents;
		ind = q;
		for(w = q; w < n; w++)
		{
			if(rands[w].contents < min)
			{
				min = rands[w].contents;
				ind = w;
			}
		}
		temp = rands[q].contents;
		rands[q].contents = min;
		rands[ind].contents = temp;

	}
}

static void one_round(const struct ssalg_multi *this, const uint8_t *inbuf, size_t len, uint8_t *const *out, size_t *outlen)
{

	element_t** xCords = calloc((this->n)+len, sizeof(xCords[0]));

	element_t** yCords = calloc((this->n)+len, sizeof(yCords[0]));


	//rands needs to be Us and Ds 
	//contains uuuuddddddd for however many ds and us.  
	gf257_element_t* rands = calloc((this->n)+len, sizeof(gf257_element_t));
	unique_rand(rands, (this->n), len);

	//sort the U's (this should make a few things easier.)
	sort(rands, this->n);
	/*
	int debug;
	for(debug = 0; debug < this->n; debug++)
	{
		printf("%d \n",rands[debug].contents);
	}
	*/


	//load the ds into rand,
	unsigned dval, check;
	check = 0;
	for (dval = 0; dval < len; dval++)
	{
		if(dval + len < rands[check].contents)
		{
			gf257_init(&rands[(this->n)+dval]);
			rands[(this->n)+dval].contents = dval+len;
		}
		else
		{
			check++;
		}
	}

	gf257_element_t* shadows = calloc((this->n), sizeof(gf257_element_t));
	unique_rand(shadows, (this->n), 0);


	unsigned counter;
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
		gf257_element_t S = shadows[i];
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

		//printf("%d\n",hashed.contents);

		//load coords.
		gf257_element_t* temp = malloc(sizeof(gf257_element_t));
		gf257_init(temp);
		temp->contents = rands[i].contents; //Ui

		//printf("%d\n", rands[i].contents);
		
		xCords[counter+i] = &temp->super;

		gf257_element_t* temp2 = malloc(sizeof(gf257_element_t));
		gf257_init(temp2);
		temp2->contents = hashed.contents;
		yCords[counter+i] = &temp2->super;

	}

	poly_t* polynomial;

	polynomial = interpolate(xCords, yCords, counter+i);
	counter = counter+i;

	//debugging code start
	/*
	printf("One round polynomial: \n");
	for(i = 0; i <= polynomial->degree; i++)
	{
		printf("%dx^%d  ", ((gf257_element_t*)polynomial->coeffs[i])->contents, i);
	}
	printf("\n");

	gf257_element_t* toEval = malloc(sizeof(gf257_element_t));
	gf257_init(toEval);
	gf257_element_t* result;

	printf("Sanity check: \n");
	for(i = 0; i < 5; i++)
	{
		toEval->contents = i;
		result = (gf257_element_t*) eval_poly(polynomial, (element_t*) toEval);
		printf("%c", result->contents);
	}
	printf("\n");
	printf("\n");

	free(toEval);
	*/
	//debugginh code stop


	//load the published values into out. 
	unsigned j;
	int k; 
	size_t bufsize;
	uint8_t dumBuf[2];

	bufsize = buf_convert(&R, dumBuf);
	memcpy(out[0], &R.contents, bufsize);
	outlen[0] += bufsize;

	//WARNING: possible error with 255/256 numbers as contents,
	//ask Dr. Pohly,
	for(k = 0; k < (this->n); k++)
	{
		//load s and u
		bufsize = buf_convert(&shadows[k], dumBuf);
		memcpy(out[k] + outlen[k], dumBuf, bufsize);
		outlen[k] += bufsize;
		printf("%d\n", rands[k].contents);
		bufsize = buf_convert(&rands[k], dumBuf);
		memcpy(out[k] + outlen[k], dumBuf, bufsize);
		outlen[k] += bufsize;
	}

	gf257_element_t* d;

	for (j = 0; j < (len); j++)
	{
		int ch = (j+1)%this->n;
		//load evaluated points
		printf("%d\n", rands[(this->n)+j].contents);
		d = (gf257_element_t*) eval_poly(polynomial, (element_t*) &rands[(this->n)+j]);
		printf("%d\n", d->contents);
		//starting with channel 1 
		bufsize = buf_convert(d, dumBuf);
		memcpy(out[ch] + outlen[ch], dumBuf, bufsize);
		outlen[ch] += bufsize;

	}

	poly_free(polynomial);
	free(rands);
	free(shadows);
	for(i = 0; i<counter; i++)
	{
		free(xCords[i]);
		free(yCords[i]);
	}
	//elements need to be freed first
	free(xCords);
	free(yCords);
	free(d);

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

	for(offset = 0; offset < len; offset+= SECRETS)
	{
		for(i = 0; i< this->n; i++)
			outlen[i] = 0;

		length = len - offset < SECRETS? len-offset: SECRETS;
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

//outlen keeps track of we are currently looking in shares
static int one_recombine(const struct ssalg *super, uint8_t *buf, size_t bufloc, size_t len, size_t *inlen, struct ss_packet **shares)
{

	const struct ssalg_multi *this = (const struct ssalg_multi *) super;

	gf257_element_t* xCords[len+(this->n)];
	gf257_element_t* yCords[len+(this->n)];

	gf257_element_t R;
	gf257_init(&R);
	inlen[0] += bytes_convert(&R, shares[0]->data[inlen[0]]);


	int i, j, counter;
	counter = 0; //index into x and y cords.


	gf257_element_t result;
	gf257_init(&result);

	//recreate the pairs of (ui, f(r, si))
	for(i=0; i<this->n; i++)
	{
		uint8_t dumBuf[2];
		SHA1_CTX hash;
		SHA1Init(&hash);
		size_t numBytes;

		//get the s and u, compute the shadow and load the points. 
		numBytes = bytes_convert(&result, shares[i]->data[inlen[i]]);

		//load the s
		SHA1Update(&hash, shares[i]->data[inlen[i]], numBytes);
		inlen[i] += numBytes;

		numBytes = buf_convert(&R, dumBuf);
		SHA1Update(&hash, dumBuf, numBytes);

		//load Ui into result
		inlen[i] += bytes_convert(&result, shares[i]->data[inlen[i]]);
		xCords[counter] = malloc(sizeof(gf257_element_t));
		gf257_init(xCords[counter]);
		xCords[counter]->contents = result.contents;

		//compute the hash
		uint8_t digest[20];
		SHA1Final(&hash, digest);

		gf257_element_t hashed;
		gf257_init(&hashed);

		//turn digest into an element.
		bytes_convert(&hashed, digest);

		yCords[counter] = malloc(sizeof(gf257_element_t));
		gf257_init(yCords[counter]);
		yCords[counter]->contents = hashed.contents;

		counter++;
	}

	//recreate the pairs of ds and h(ds)
	int uCheck, channel, d;
	uCheck = 0;
	d = 0; //d manages the gaps created by the fact ds and us cannot overlap
	// thus the d value will be i+len+d
	for(i = 0; i < len; i++)
	{
		channel = (i+1)%(this->n);

		//load d
		xCords[counter] = malloc(sizeof(gf257_element_t));
		gf257_init(xCords[counter]);
		//make sure d not a u
		if((i+len+d)<xCords[uCheck]->contents)
			xCords[counter]->contents = i+len+d;
		else
		{
			//in case successive us
			while((i+len+d)==xCords[uCheck]->contents)
			{
				d++;
				uCheck++;
			}
			xCords[counter]->contents = i+len+d;
		}
		

		//load h(d)
		inlen[channel] += bytes_convert(&result, shares[channel]->data[inlen[channel]]);
		yCords[counter] = malloc(sizeof(gf257_element_t));
		gf257_init(yCords[counter]);
		yCords[counter]->contents = result.contents;

		counter++;
	}

	poly_t* interpt;

	interpt = interpolate((element_t**) xCords, (element_t**) yCords, len+(this->n));

	/*
	//load the decoded secrets into buf
	gf257_element_t* toEval = malloc(sizeof(gf257_element_t));
	gf257_init(toEval);
	for (i = 0; i < len; i++)
	{
		toEval->contents = i;
		result = eval_poly(interpt, (element_t*) toEval);
	}
	*/
	

	return 0;
}



//Required by ssalg.h.  Currently in dummy form.
static ssize_t ssalg_multi_recombine(struct ssalg *super, uint8_t *buf, size_t len, struct ss_packet **shares)
{
	const struct ssalg_multi *this = (const struct ssalg_multi *) super;

	int offset, i; 
	size_t bufloc, length; //location in buf
	size_t inlen[this->n]; //location on each channel
	bufloc = 0;

	for(offset = 0; offset < len; offset+= SECRETS)
	{
		for(i = 0; i< this->n; i++)
			inlen[i] = 0;

		//length is the number of secrets involved in the next decoding round
		length = len - offset < SECRETS? len-offset: SECRETS;

		one_recombine(this, buf, bufloc, length, inlen, shares);

	}

	return 0;
}



//Required by ssalg.h, based on ssalg_xor.
static const struct ssalg_ops ssalg_multi_ops = {
	.get_n = ssalg_multi_get_n,
	.get_k = ssalg_multi_get_k,
	.split = ssalg_multi_split,
	.recombine = ssalg_multi_recombine,
	.destroy = NULL, //This probably should be defined eventually (before testing)
};

