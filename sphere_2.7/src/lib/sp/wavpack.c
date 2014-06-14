/* LINTLIBRARY */

#ifndef lint
    static char copyright[]=
"***************************************************************************\n\
	Copyright (C) 1992 Massachusetts Institute of Technology. \n\
			All rights reserved \n\
***************************************************************************\n";
#endif

/*
 * This software is being provided to you, the LICENSEE, by the Massachusetts
 * Institute of Technology (M.I.T.) under the following license.  By
 * obtaining, using and/or copying this software, you agree that you have
 * read, understood, and will comply with these terms and conditions:  
 *  
 * Permission to use, copy, modify and distribute, including the right to
 * grant others the right to distribute at any tier, this software and its
 * documentation for any purpose and without fee or royalty is hereby granted,
 * provided that you agree to comply with the following copyright notice and
 * statements, including the disclaimer, and that the same appear on ALL
 * copies of the software and documentation, including modifications that you
 * make for internal use or for distribution:
 *
 * Copyright 1992 by the Massachusetts Institute of Technology.  All rights
 * reserved. 
 *
 * THIS SOFTWARE IS PROVIDED "AS IS", AND M.I.T. MAKES NO REPRESENTATIONS OR
 * WARRANTIES, EXPRESS OR IMPLIED.  By way of example, but not limitation,
 * M.I.T. MAKES NO REPRESENTATIONS OR WARRANTIES OF MERCHANTABILITY OR FITNESS
 * FOR ANY PARTICULAR PURPOSE OR THAT THE USE OF THE LICENSED SOFTWARE OR
 * DOCUMENTATION WILL NOT INFRINGE ANY THIRD PARTY PATENTS, COPYRIGHTS,
 * TRADEMARKS OR OTHER RIGHTS.   
 *  
 * The name of the Massachusetts Institute of Technology or M.I.T. may NOT be
 * used in advertising or publicity pertaining to distribution of the
 * software.  Title to copyright in this software and any associated
 * documentation shall at all times remain with M.I.T., and USER agrees to
 * preserve same. 
 */

#ifndef lint
    static char rcsid[] = "$Header: /home/sware/cvs/SPHERE/nist/src/lib/sp/wavpack.c,v 1.1.1.1 2003/07/22 14:03:39 jon Exp $";
#endif

/* wavpack: machine indpendent waveform packing routine
 * assumes only that data is organized in bytes
 * tries a number of strategies: includes mono/stereo, byte/short, data
 * lossless (bit preserving) for any kind of input data
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Setjmp added by JGF */
# include <setjmp.h>
extern jmp_buf	exitenv;

#include <sp/sphere.h>
#include <sp/wavpack/wavpack.h>


static char magic[] = WAVPACK_MAGIC;	/* magic string to identify wavpack files */

static int strat[]=		/* strategy info */
{	BYTE,				/* 0 */
	BYTE|DIFF,			/* 1 */
	BYTE|STEREO,			/* 2 */
	BYTE|DIFF|STEREO,		/* 3 */

	SHORT,				/* 4 */
	SHORT|REV,			/* 5 */
	SHORT|DIFF,			/* 6 */
	SHORT|REV|DIFF,			/* 7 */
	SHORT|OFFSET,			/* 8 */
	SHORT|OFFSET|REV,		/* 9 */
	SHORT|OFFSET|DIFF,		/* 10 */
	SHORT|OFFSET|REV|DIFF,		/* 11 */
	SHORT|STEREO,			/* 12 */
	SHORT|REV|STEREO,		/* 13 */
	SHORT|DIFF|STEREO,		/* 14 */
	SHORT|REV|DIFF|STEREO,		/* 15 */
	SHORT|OFFSET|STEREO,		/* 16 */
	SHORT|OFFSET|REV|STEREO,	/* 17 */
	SHORT|OFFSET|DIFF|STEREO,	/* 18 */
	SHORT|OFFSET|REV|DIFF|STEREO,	/* 19 */
	-1
};
static int nstrat;

struct hdr			/* compressed data header */
{	unsigned char flags;		/* strategy and channel info */
	unsigned char count;		/* nr samples -1 */
	unsigned char nbits;		/* nr pcm bits */
	unsigned char offset_byte;	/* (opt) offst byte */
	unsigned short dbase;		/* (opt) diff base, byte/short */
	unsigned short pbase;		/* (opt) pcm base, byte/short */
};

static struct strat_eval	/* strat evaluation with sample hdrs */
{	struct hdr hdr;

	int nsam;	/* samples available (from condition_data) */

	int min, max;	/* min and max of data */
	int range;	/* range of data */
	int count;	/* nr samples */
	int obytes;	/* total output bytes (including header) */
	int ibytes;	/* input bytes consumed */
	float obpib;	/* output bytes per input byte */

	int chan_1;	/* if chan0, stereo pair (chan1) */
} strat_eval[256];	/* address microcoded through strat[] */

static struct strat_eval f0b[RUNL+1],f1b[RUNL+1];	/* for mixed strat stereo */
static struct strat_eval f0s[RUNL+1],f1s[RUNL+1];			
static struct strat_eval f0so[RUNL+1],f1so[RUNL+1];			

static float bm,b0b,b1b,b0s,b1s,b0so,b1so;		/* bests for pruning */

static unsigned char input[4*RUNL+5];		/* raw input */
static int ninput;
					/* input channels for strategies */
static unsigned short chan_0[RUNL];		/* byte/short chan 0 */
static unsigned short chan_1[RUNL];		/* byte/short chan 1 */

static int vflg=0;		/* verbosity */
static int monoflg = 0;		/* mono data only */
static int byteflg = 0;		/* byte data only */
static int sflg= -1;
static int runl=128;		/* max run length */

static char *progname = (char *) NULL;
static char *oname = (char *) NULL;
static int oflg = 0;		/* if non-zero, unlink(oname) on error */

static int pwr2[17]={1,2,4,8,16,32,64,128,256,
	512,1024,2048,4096,8192,16384,32678,65536};


#define mod(x) ((x)&bitmask)
/* #define maxm(x,y) ((x)>(y)?(x):(y)) */

#define perr(s)			wavpack_perr(s)

void wavpack_set_monoflg(int val)
{
	monoflg = val;
}

int wavpack_get_monoflg(void)
{
	return monoflg;
}

void wavpack_set_byteflg(int val)
{
	byteflg = val;
}

int wavpack_get_byteflg(void)
{
	return byteflg;
}

int wavpack_set_vflg(int value)
{
	vflg = value;
}

int wavpack_set_oname(char *s)
{
	if ( oname != (char *) NULL )
		free( oname );
	oname = mtrf_strdup( s );
	return (oname == (char *) NULL) ? -1 : 0;
}

void wavpack_set_oflg(int value)
{
	oflg = value;
}

int wavpack_set_progname(char *s)
{
	if ( progname != (char *) NULL )
		free( progname );
	progname = mtrf_strdup( s );
	return ( progname == (char *) NULL ) ? -1 : 0;
}

int wavpack_free_progname(void)
{
	if ( progname != (char *) NULL )
		mtrf_free( progname );
	progname = (char *)NULL;
	return ( progname != (char *) NULL ) ? -1 : 0;
}

void wavpack_dump_interface(FILE *fp)
{
    fprintf(fp,"Wavpack: monoflg=%d\n",monoflg);
    fprintf(fp,"Wavpack: byteflg=%d\n",byteflg);
    fprintf(fp,"Wavpack: vflg=%d\n",vflg);
    fprintf(fp,"Wavpack: oflg=%d\n",oflg);
    fprintf(fp,"Wavpack: oname=%s\n",oname);
    fprintf(fp,"Wavpack: progname=%s\n",progname);
}

int wavpack_get_nstrat(void)
{
	int n;

	for ( n=0; strat[n] >= 0; n++ ) ;
	return n;
}

#ifdef FILE
#	undef FILE
#endif
#define FILE FOB

#ifdef fread
#	undef fread
#endif
#define fread(a,b,c,d)		fob_fread((void *)(a),(b),(c),(d))

#ifdef fwrite
#	undef fwrite
#endif
#define fwrite(a,b,c,d)		fob_fwrite((void *)(a),(b),(c),(d))

#ifdef ferror
#	undef ferror
#endif
#define ferror(a)		fob_ferror((a))


static evaluate_strat(int flags);
static evaluate_chan(int flags, struct strat_eval *f);
static output_strat(int flags, FILE *ofile);
static output_chan(int flags, FILE *ofile);
static int readchan(FILE *ifile, struct hdr *hp, unsigned char *ibuf);
static decode_data(struct hdr *hp, unsigned char *ibuf, register short unsigned int *sobuf, int *nsobuf);
static write_data(struct hdr *hdr, short unsigned int (*sobuf)[257], int *nsobuf, FILE *ofile);
static printstrat(int x);


int wavpack_pack(FILE *ifile, FILE *ofile)
{	register i,j,k,n;
	int m,eof,ibytes,obytes,inbytes,flags;
	int hist[256];
	float obpib,x,*bp;

	nstrat = wavpack_get_nstrat();
								/* magic */
	if(fwrite(magic,strlen(magic)+1,1,ofile)!=1/*==0*/ ) perr("fwrite err");

	for(i=0;i<256;i++) hist[i]=0;

	obytes=strlen(magic)+1;
	ibytes=inbytes=ninput=eof=0;
	for(m=0;;m++)
	{	if(!eof)
		{	n=fread(&input[ninput],1,4*RUNL+5-ninput,ifile);
			if(n==0)
			{	if(ferror(ifile)) perr("input file error");
				eof=1;
				if(ninput==0) break;
			}
			ninput+=n;
			inbytes+=n;
		}

		for(i=0;i<nstrat;i++)
		{	strat_eval[strat[i]].obpib=1e5;
			if(strat[i]&STEREO)
				strat_eval[strat[i]|CHAN1].obpib=1e5;
		}
		if(!monoflg) for(i=0;i<runl+1;i++)
			f0b[i].obpib=f1b[i].obpib=1e5;
		if(!byteflg && !monoflg)
			for(i=0;i<runl+1;i++) f0s[i].obpib=f1s[i].obpib=
				f0so[i].obpib=f1so[i].obpib=1e5;
		bm=b0b=b1b=b0s=b1s=b0so=b1so=1e5;

		if(sflg>=0) evaluate_strat(strat[sflg]);	/* debug */
		else for(i=0;i<nstrat;i++)
		{	flags=strat[i];
			if(monoflg && (flags&STEREO)) continue;
			if(byteflg && (flags&SHORT)) continue;
			evaluate_strat(flags);

			if(flags&STEREO)
			{	if(flags&SHORT)
				{	if(flags&CHAN1)
					{	if(flags&OFFSET) bp= &b1so;
						else bp= &b1s;
					}
					else
					{	if(flags&OFFSET) bp= &b0so;
						else bp= &b0s;
					}
				}
				else
				{	if(flags&CHAN1) bp= &b1b;
					else bp= &b0b;
				}
			}
			else bp= &bm;
			if(strat_eval[flags].obpib<*bp)
				*bp=strat_eval[flags].obpib;
		}

		if(!monoflg)				/* finish stereo */
		{	for(i=0;i<nstrat;i++) if(strat[i]&STEREO)   
				strat_eval[strat[i]].obpib=1e5;
			for(i=0,j= -1,x=1000;i<runl+1;i++)	/* byte */
			{	if(f0b[i].obpib+f1b[i].obpib<10.)
				{	f0b[i].obytes+=f1b[i].obytes;
					f0b[i].ibytes+=f1b[i].ibytes;
					f0b[i].obpib=(float)
						f0b[i].obytes/f0b[i].ibytes;
					if(f0b[i].obpib<x) x=f0b[i].obpib, j=i;
					f0b[i].chan_1=f1b[i].hdr.flags;
				}
			}
			if(j>=0)
			{	strat_eval[f0b[j].hdr.flags]=f0b[j];
				strat_eval[f1b[j].hdr.flags]=f1b[j];
			}
		}

		if(!monoflg && !byteflg)		/* finish stereo */
		{	for(i=0,j= -1,x=1000;i<runl+1;i++)	/* short */
			{	if(f0s[i].obpib+f1s[i].obpib<10.)
				{	f0s[i].obytes+=f1s[i].obytes;
					f0s[i].ibytes+=f1s[i].ibytes;
					f0s[i].obpib=(float)
						f0s[i].obytes/f0s[i].ibytes;
					if(f0s[i].obpib<x) x=f0s[i].obpib, j=i;
					f0s[i].chan_1=f1s[i].hdr.flags;
				}
			}
			if(j>=0)
			{	strat_eval[f0s[j].hdr.flags]=f0s[j];
				strat_eval[f1s[j].hdr.flags]=f1s[j];
			}

			for(i=0,j= -1,x=1000;i<runl+1;i++)   /* short, offset */
			{	if(f0so[i].obpib+f1so[i].obpib<10.)
				{	f0so[i].obytes+=f1so[i].obytes;
					f0so[i].ibytes+=f1so[i].ibytes;
					f0so[i].obpib=(float)
						f0so[i].obytes/f0so[i].ibytes;
					if(f0so[i].obpib<x) x=f0so[i].obpib,j=i;
					f0so[i].chan_1=f1so[i].hdr.flags;
				}
			}
			if(j>=0)
			{	strat_eval[f0so[j].hdr.flags]=f0so[j];
				strat_eval[f1so[j].hdr.flags]=f1so[j];
			}
		}

		k= -1;				/* choose winning strategy */
		obpib=100;
		for(i=0;i<nstrat;i++) if(strat_eval[strat[i]].obpib<obpib)
		{	k=strat[i];
			obpib=strat_eval[k].obpib;
		}
		if(obpib<=0) perr("pack: illegal obpib");
		if(k<0) perr("pack: no channel found");
		if(sflg>=0 && k!=strat[sflg]) perr("k!=strat[sflg]");
		hist[k]++;

		output_strat(k,ofile);

		if(vflg>2)
		{	fprintf(stderr,"%d\t",m);
			fprintf(stderr,"%d\t",ibytes);
			printstrat(k);
			fprintf(stderr,"\tib=%d\tob=%d",
				strat_eval[k].ibytes,strat_eval[k].obytes);
			fprintf(stderr,"\t%d",strat_eval[k].hdr.nbits);
			fprintf(stderr,"\t%.2f",strat_eval[k].obpib);
			fprintf(stderr,"\n");
			if(k&STEREO)
			{	fprintf(stderr,"%d\t",m);
				fprintf(stderr,"%d\t",ibytes);
				j=strat_eval[k].chan_1;
				printstrat(j);
				fprintf(stderr,"\tib=%d\tob=%d",
				    strat_eval[j].ibytes,strat_eval[j].obytes);
				fprintf(stderr,"\t%d",strat_eval[j].hdr.nbits);
				fprintf(stderr,"\t%.2f",strat_eval[j].obpib);
				fprintf(stderr,"\n");
			}
		}

		j=strat_eval[k].ibytes;			/* shift input */
		for(i=0;j<ninput;i++,j++) input[i]=input[j];
		ninput-=strat_eval[k].ibytes;
		if(ninput<0) perr("ninput<0");

		ibytes+=strat_eval[k].ibytes;
		obytes+=strat_eval[k].obytes;
		if(eof && ninput==0) break;
	}
	if(ibytes!=inbytes) perr("ibytes!=inbytes");

	if(vflg)
	{	if(vflg>1)
		{	fprintf(stderr,"ib=%d\tob=%d\t",ibytes,obytes);
			fprintf(stderr,"rec=%d\t",m);
		}
		fprintf(stderr,"compression=%.1f%%\n",100.-100.*obytes/ibytes);

		if(vflg>1)
		{	for(i=0;i<256;i++) if(hist[i])
			{	printstrat(i);
				fprintf(stderr,"\t%d\t",hist[i]);
			}
			fprintf(stderr,"\n");
		}
	}
}

static int condition_data(int flags)
{	register int i,j,n;
	int osam,nbytes;
	unsigned short sinput[4*RUNL+5];

	if(flags&CHAN1) perr("flags&CHAN1");
	strat_eval[flags].hdr.flags=flags;

	nbytes=0;	/* input bytes consumed */
	osam=0;		/* ouput samples generated */
	if(flags&SHORT)						/* short data */
	{	if(flags&OFFSET)
		{	strat_eval[flags].hdr.offset_byte=input[nbytes++];
			if(nbytes>=ninput) return 1;
		}

		i=nbytes;			/* construct short array */
		if(flags&REV)			/* rev (lsByte first) */
		{	for(j=0;i<ninput;i+=2,j++) sinput[j]=
				((input[i])&0377) | (((input[i+1])&0377)<<8);
		}
		else				/* normal (msByte first) */
		{	for(j=0;i<ninput;i+=2,j++) sinput[j]=
				(((input[i])&0377)<<8) | ((input[i+1])&0377);
		}

		n=0;
		if(flags&STEREO)				/* stereo */
		{	if(flags&DIFF)				/* diff */
			{	strat_eval[flags].hdr.dbase=sinput[n++];
				nbytes+=2;
				if(nbytes>=ninput) return 1;
				strat_eval[flags+CHAN1].hdr.dbase=sinput[n++];
				nbytes+=2;
				if(nbytes+4>ninput) return 1;
				while(osam<runl && nbytes+4<=ninput)
				{	chan_0[osam]=sinput[n]-sinput[n-2];
					n++;
					chan_1[osam++]=sinput[n]-sinput[n-2];
					n++;
					nbytes+=4;
				}
			}
			else					/* pcm */
			{	if(nbytes+4>ninput) return 1;
				while(osam<runl && nbytes+4<=ninput)
				{	chan_0[osam]=sinput[n++];
					chan_1[osam++]=sinput[n++];
					nbytes+=4;
				}
			}
		}
		else						/* mono */
		{	if(flags&DIFF)				/* diff */
			{	strat_eval[flags].hdr.dbase=sinput[n++];
				nbytes+=2;
				if(nbytes+2>ninput) return 1;
				while(osam<runl && nbytes+2<=ninput)
				{	chan_0[osam++]=
						sinput[n]-sinput[n-1];
					n++;
					nbytes+=2;
				}
			}
			else					/* pcm */
			{	if(nbytes+2>ninput) return 1;
				while(osam<runl && nbytes+2<=ninput)
				{	chan_0[osam++]=sinput[n++];
					nbytes+=2;
				}
			}
		}
	}
	else							/* byte data */
	{	if(flags&OFFSET) perr("byte && offset");
		if(flags&REV) perr("byte && byte_rev");

		if(flags&STEREO)				/* stereo */
		{	if(flags&DIFF)				/* diff */
			{	strat_eval[flags].hdr.dbase=input[nbytes++];
				if(nbytes>=ninput) return 1;
				strat_eval[flags|CHAN1].hdr.dbase=
					input[nbytes++];
				if(nbytes+2>ninput) return 1;
				while(osam<runl && nbytes+2<=ninput)
				{	chan_0[osam]=
						input[nbytes]-input[nbytes-2];
					nbytes++;
					chan_1[osam++]=
						input[nbytes]-input[nbytes-2];
					nbytes++;
				}
			}
			else					/* pcm */
			{	if(nbytes+2>ninput) return 1;
				while(osam<runl && nbytes+2<=ninput)
				{	chan_0[osam]=input[nbytes++];
					chan_1[osam++]=input[nbytes++];
				}
			}
		}
		else						/* mono */
		{	if(flags&DIFF)				/* diff */
			{	strat_eval[flags].hdr.dbase=input[nbytes++];
				if(nbytes+1>ninput) return 1;
				while(osam<runl && nbytes+1<=ninput)
				{	chan_0[osam++]=
						input[nbytes]-input[nbytes-1];
					nbytes++;
				}
			}
			else					/* pcm */
			{	if(nbytes+1>ninput) return 1;
				while(osam<runl && nbytes+1<=ninput)
					chan_0[osam++]=input[nbytes++];
			}
		}
	}
	strat_eval[flags].nsam=osam;
	if(flags&STEREO) strat_eval[flags|CHAN1].nsam=osam;
	return 0;
}

static evaluate_strat(int flags)
{	register int i,count;
	register struct strat_eval *f0p,*f1p;			
	int flags1;
	struct strat_eval f0[RUNL],f1[RUNL];			

	strat_eval[flags].obpib=1e5;

	if(flags&CHAN1) perr("flags&CHAN1");

	if(condition_data(flags)) return 1;

	if(flags&STEREO)
	{	flags1=flags|CHAN1;
		if(strat_eval[flags].nsam!=strat_eval[flags1].nsam)
			perr("STEREO: nsam!=");

		if(flags&SHORT && flags&OFFSET) f0p= &f0so[0], f1p= &f1so[0];
		else if(flags&SHORT) f0p= &f0s[0], f1p= &f1s[0];
		else f0p= &f0b[0], f1p= &f1b[0];

		if(flags&DIFF) f0p++, f1p++;		/* offset for DIFF */

		evaluate_chan(flags,f0);
		count=strat_eval[flags].count;
		for(i=0;i<count;i++) if(f0[i].obpib<f0p[i].obpib) f0p[i]=f0[i];

		evaluate_chan(flags1,f1);
		count=strat_eval[flags1].count;
		for(i=0;i<count;i++) if(f1[i].obpib<f1p[i].obpib) f1p[i]=f1[i];

		/* must be finished in pack() */
	}
	else evaluate_chan(flags,f0);
}

static evaluate_chan(int flags, struct strat_eval *f)		/* evaluate channel n */
                     			
{	register i,j,s;
	unsigned short *chanp;
	int dmin,dmax;
	int chan,hbytes,obits,pflg;
	int nsam,bitmask,maxbits,bpw;
	struct strat_eval x,y;
	float obpib,best;

	nsam=strat_eval[flags].nsam;
	if(flags&SHORT) bitmask=0xffff, maxbits=16, bpw=2;	/* 16 bit */
	else bitmask=0xff, maxbits=8, bpw=1;			/* 8 bit */

	if(flags&STEREO)
	{	if(flags&SHORT)
		{	if(flags&CHAN1)
			{	if(flags&OFFSET) best=b1so;
				else best=b1s;
			}
			else
			{	if(flags&OFFSET) best=b0so;
				else best=b0s;
			}
		}
		else
		{	if(flags&CHAN1) best=b1b;
			else best=b0b;
		}
	}
	else best=bm;

	if(flags&CHAN1) chanp=chan_1, chan=1;
	else chanp=chan_0, chan=0;

	x=strat_eval[flags];
	x.hdr.flags=flags;
	x.hdr.count=0;
	x.hdr.nbits=0;
	x.hdr.pbase=chanp[0];

	x.min=x.max=chanp[0];
	x.range=0;
	x.count=0;
	x.obytes=0;
	x.ibytes=0;
	x.obpib=1000;

	hbytes=3;				/* req header bytes */
	if(chan==0 && flags&OFFSET) hbytes++;	/* for offset_byte */
	if(flags&DIFF) hbytes+=bpw;		/* for dbase */
	hbytes+=bpw;				/* for pbase if(nbits<maxbits)*/
	x.obytes=hbytes;
	obits=7;

	x.ibytes=0;				/* initial input bytes */
	if(chan==0 && flags&OFFSET) x.ibytes++;	/* for offset_byte */
	if(flags&DIFF) x.ibytes+=bpw;		/* for dbase */
	obpib=0.;

	y=x;
	for(i=pflg=0;i<nsam;i++)
	{	s=chanp[i];	/* current sample */
		if(x.hdr.nbits<maxbits)
		{	dmin=mod(s-x.min);		/* outside zone */
			dmax=mod(x.max-s);
			if(dmin>x.range || dmax>x.range)
			{	if(dmax>=dmin) x.max=s;
				else x.min=x.hdr.pbase=s;
				x.range=mod(x.max-x.min);
	
				if(x.range>=pwr2[x.hdr.nbits])
				{	for(j=x.hdr.nbits;j<17;j++)
						if(x.range<pwr2[j]) break;
					if(j==maxbits)
					{	x.min=x.hdr.pbase=0;
						x.max=x.range=bitmask;
						hbytes-=bpw;
					}
					x.hdr.nbits=j;	/* bits req for pcm */
					obits=j*x.count+7;    /* output bits */
					x.obytes=hbytes+obits/8; /* and bytes */
					obits%=8;
					obpib=(float)j/maxbits;
					if(obpib>y.obpib) break;
					if(obpib>best) pflg=1;
				}
			}
		}
		x.count++;
		x.hdr.count=x.count-1;

		for(obits+=x.hdr.nbits;obits>=8;obits-=8) x.obytes++; /*output*/
		x.ibytes+=bpw;				/* input bytes */
		x.obpib=(float)x.obytes/x.ibytes;	/* obytes per ibytes */

		f[i]=x;				/* save result */
		if(x.obpib<y.obpib)
		{	y=x;			/* best forward coding so far */
			if(obpib>y.obpib) break;	/* can only get worse */
		}
		if(pflg && x.obpib>best) break;	/* occasional premature prune */
	}
	strat_eval[flags]=y;		/* best run for this coding */
}

static output_strat(int flags, FILE *ofile)
{	if(flags&CHAN1) perr("output: flags&CHAN1");

	if(condition_data(flags)) perr("output: unable to make channel");
	output_chan(flags,ofile);

	if(flags&STEREO)
	{	flags=strat_eval[flags].chan_1;
		if(condition_data(flags&~CHAN1))
			perr("output: unable to make channel");
		output_chan(flags,ofile);
	}
}

static output_chan(int flags, FILE *ofile)
{	register int i,j,k,pbase;
	register unsigned short *chanp;
	int bits,count;
	unsigned char c[2],obuf[4*RUNL+5];

	bits=strat_eval[flags].hdr.nbits;
	count=strat_eval[flags].hdr.count+1;

	if(strat_eval[flags].hdr.flags!=flags) perr("flags!=");
	if(flags&CHAN1) chanp=chan_1;
	else chanp=chan_0;

	if(fwrite(&strat_eval[flags].hdr.flags,1,1,ofile)!=1 /*==0*/ )	/* header */
		perr("fwrite err");
	if(fwrite(&strat_eval[flags].hdr.count,1,1,ofile)!=1 /*==0*/ )	/* header */
		perr("fwrite err");
	if(fwrite(&strat_eval[flags].hdr.nbits,1,1,ofile)!=1 /*==0*/ )	/* header */
		perr("fwrite err");
	if(flags&OFFSET && (flags&CHAN1)==0 &&			/* offset */
		fwrite(&strat_eval[flags].hdr.offset_byte,1,1,ofile)!=1 /*==0*/ )
			perr("fwrite err");
	if(flags&DIFF)						/* diff base */
	{	if(flags&SHORT)
		{	c[0]=strat_eval[flags].hdr.dbase>>8;
			c[1]=strat_eval[flags].hdr.dbase;
			if(fwrite(c,1,2,ofile)!=2 /*==0*/ ) perr("fwrite err");
		}
		else
		{	c[0]=strat_eval[flags].hdr.dbase;
			if(fwrite(c,1,1,ofile)!=1 /*==0*/ ) perr("fwrite err");
		}
	}
	if(bits<8 || (flags&SHORT && bits<16))			/* pcm base */
	{	if(flags&SHORT)
		{	c[0]=strat_eval[flags].hdr.pbase>>8;
			c[1]=strat_eval[flags].hdr.pbase;
			if(fwrite(c,1,2,ofile)!=2 /*==0*/ ) perr("fwrite err");
		}
		else
		{	c[0]=strat_eval[flags].hdr.pbase;
			if(fwrite(c,1,1,ofile)!=1 /*==0*/ ) perr("fwrite err");
		}
		pbase=strat_eval[flags].hdr.pbase;
	}
	else pbase=0;
	
	if(flags&CHAN1) chanp=chan_1;
	else chanp=chan_0;

	if(bits>0)
	{	if(pbase) for(i=0;i<count;i++) chanp[i]-=pbase;
		for(i=0;i<(bits*count+7)/8;i++) obuf[i]=0;
#if 0
		for(i=0;i<bits*count;i++)
		{	if((1<<(bits-1-i%bits))&chanp[i/bits])
				obuf[i/8]|=1<<(7-i%8);
		}
#endif
		for(i=k=0;i<count;i++) for(j=bits-1;j>=0;j--,k++)
			if((1<<j)&chanp[i]) obuf[k>>3]|=0x80>>(k&7);

		{ int nitems;
		nitems = (bits*count+7)/8;
		if(fwrite(obuf,1,nitems,ofile)!=nitems /*==0*/ )	/* pcm data */
			perr("fwrite err");
		}
	}
}

int wavpack_unpack(FILE *ifile, FILE *ofile)
{	int k;
	char str[100];
	unsigned char ibuf[2][4*RUNL+5];
	unsigned short sobuf[2][RUNL+1];
	int nsobuf[2];
	struct hdr hdr[2];

	nstrat = wavpack_get_nstrat();

	if(fread(str,strlen(magic)+1,1,ifile)==0)		/* read magic */
		perr("fread err");
	if(strcmp(str,magic)!=0) perr("not a wavpacked file");
	for(k=0;;k++)
	{	if(vflg>1) fprintf(stderr,"k=%d\t",k);

		if(readchan(ifile,&hdr[0],ibuf[0])) break;
		if(vflg>1)
		{	printstrat((int)hdr[0].flags);
			fprintf(stderr,"\n");
			if(hdr[0].flags&STEREO)
			{	fprintf(stderr,"k=%d\t",k);
				printstrat((int)hdr[1].flags);
				fprintf(stderr,"\n");
			}
		}
		decode_data(&hdr[0],ibuf[0],sobuf[0],&nsobuf[0]);
		if(hdr[0].flags&STEREO)
		{	if(readchan(ifile,&hdr[1],ibuf[1]))
				perr("illegal chan 1");
			decode_data(&hdr[1],ibuf[1],sobuf[1],&nsobuf[1]);
		}
		write_data(hdr,sobuf,nsobuf,ofile);
	}
	if(vflg>1) fprintf(stderr,"\n");
}

static int readchan(FILE *ifile, struct hdr *hp, unsigned char *ibuf)
{	int i;
	int count;
	unsigned char c[2];

	if(fread(&hp->flags,1,1,ifile)==0 /*NULL*/ ) return 1;	/* header */
	if(fread(&hp->count,1,1,ifile)==0 /*NULL*/ )		/* header */
		perr("incomplete header");
	if(fread(&hp->nbits,1,1,ifile)==0 /*NULL*/ )		/* header */
		perr("incomplete header");
	for(i=0;i<nstrat;i++) if((hp->flags&~CHAN1)==strat[i]) break;
	if(i>=nstrat) perr("illegal header");
	count=hp->count+1;

	if(hp->flags&OFFSET && (hp->flags&CHAN1)==0)	/* offset */
	{	if(fread(&hp->offset_byte,1,1,ifile)==0 /*NULL*/ )
			perr("incomplete header");
	}
	if(hp->flags&DIFF)				/* diff base */
	{	if(hp->flags&SHORT)
		{	if(fread(c,1,2,ifile)==0 /*NULL*/ )
				perr("incomplete header");
			hp->dbase=(c[0]<<8) | c[1];
		}
		else
		{	if(fread(c,1,1,ifile)==0 /*NULL*/ )
				perr("incomplete header");
			hp->dbase=c[0];
		}
	}
	if(hp->nbits<8 || (hp->flags&SHORT && hp->nbits<16)) /* pcm */
	{	if(hp->flags&SHORT)
		{	if(fread(c,1,2,ifile)==0 /*NULL*/ )
				perr("incomplete header");
			hp->pbase=(c[0]<<8) | c[1];
		}
		else
		{	if(fread(c,1,1,ifile)==0 /*NULL*/ )
				perr("incomplete header");
			hp->pbase=c[0];
		}
	}
	else hp->pbase=0;

	if(hp->nbits>0)
	{	if(fread(ibuf,(hp->nbits*count+7)/8,1,ifile)==0 /*NULL*/ )
			perr("incomplete data");
	}
	return 0;
}

static decode_data(struct hdr *hp, unsigned char *ibuf, register short unsigned int *sobuf, int *nsobuf)
{	register int i,j,k,bits,count,pbase;

	count=hp->count+1;
	bits=hp->nbits;

	if(hp->flags&DIFF)
	{	sobuf[0]=hp->dbase;
		sobuf++;
	}

	pbase=hp->pbase;
	for(i=0;i<count;i++) sobuf[i]=pbase;
#if 0
	if(bits>0) for(i=0;i<bits*count;i++) if(1<<(7-i%8)&ibuf[i/8])
			sobuf[i/bits]+=1<<(bits-1-i%bits);
#endif
	if(bits>0)
	{	for(i=k=0;i<count;i++) for(j=bits-1;j>=0;j--,k++)
			if(ibuf[k>>3]&(0x80>>(k&7))) sobuf[i]+=1<<j;
	}

	if(hp->flags&DIFF)
	{	for(i=0;i<count;i++) sobuf[i]+=sobuf[i-1];
		*nsobuf=count+1;
	}
	else *nsobuf=count;
}

static write_data(struct hdr *hdr, short unsigned int (*sobuf)[257], int *nsobuf, FILE *ofile)
{	int i,k;
	unsigned char obuf[4*RUNL+5];

	k=0;
	if(hdr[0].flags&OFFSET) obuf[k++]=hdr[0].offset_byte;

	if(nsobuf[0]>RUNL+1) perr("nsobuf>RUNL+1");
	if(hdr[0].flags&STEREO)
	{	if(nsobuf[0]!=nsobuf[1]) perr("nsobuf !=");
		if(hdr[0].flags&SHORT)
		{	for(i=0;i<nsobuf[0];i++)
			{	if(hdr[0].flags&REV)
				{	obuf[k++]=sobuf[0][i];
					obuf[k++]=sobuf[0][i]>>8;
				}
				else
				{	obuf[k++]=sobuf[0][i]>>8;
					obuf[k++]=sobuf[0][i];
				}
				if(hdr[1].flags&REV)
				{	obuf[k++]=sobuf[1][i];
					obuf[k++]=sobuf[1][i]>>8;
				}
				else
				{	obuf[k++]=sobuf[1][i]>>8;
					obuf[k++]=sobuf[1][i];
				}
			}
		}
		else for(i=0;i<nsobuf[0];i++)
		{	obuf[k++]=sobuf[0][i];
			obuf[k++]=sobuf[1][i];
		}
	}
	else
	{	if(hdr[0].flags&SHORT) for(i=0;i<nsobuf[0];i++)
		{	if(hdr[0].flags&REV)
			{	obuf[k++]=sobuf[0][i];
				obuf[k++]=sobuf[0][i]>>8;
			}
			else
			{	obuf[k++]=sobuf[0][i]>>8;
				obuf[k++]=sobuf[0][i];
			}
		}
		else for(i=0;i<nsobuf[0];i++) obuf[k++]=sobuf[0][i];
	}
	if(fwrite(obuf,1,k,ofile)!=k /*==0*/ ) perr("fwrite err");
}

static printstrat(int x)
{	int i;
	char str[9];

	for(i=0;i<5;i++) str[i]='_';
	str[5]='\0';
	
	if(x&SHORT) str[0]='S';
	else str[0]='B';
	if(x&REV) str[1]='R';
	if(x&OFFSET) str[2]='O';
	if(x&DIFF) str[3]='D';
	if(x&STEREO) str[4]='2';
	fprintf(stderr,"%s",str);
}

perr(s)
char *s;
{	if(oflg && unlink(oname))		/* delete the output file */
		fprintf(stderr,"%s err: unable to remove output file\n",
			progname);

	fprintf(stderr,"%s err: %s\n",progname,s);
	/* Removed by JGF to allow error recovery */
	/* exit(1); */
	longjmp(exitenv, 1);
}
