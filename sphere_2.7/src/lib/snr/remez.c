
/* remez.c */
/* converted from Pascal */

#include <snr/snrlib.h>

#define PI 3.1415926535

#define FOR(i,from,to) for(i=from;i<=to;i++)
#ifdef MOD
#undef MOD
#define MOD(x,y) (int) fmod((double) (x),(double) (y))
#endif

#ifdef DIV
#undef DIV
#define DIV(x,y) (int) x/y
#endif

#ifdef INT
#undef INT
#define INT(x) (int) floor((double) (x))
#endif

#ifdef TRUNC
#undef TRUNC
#define TRUNC(x) floor((double) (x))
#endif

#define NFMAX 256  /* maximum number of taps */
#define WIDTH 0.02
#define DELTA_F WIDTH/2.0

static int iext[NFMAX/2+2],nfcns,ngrid;
static double ad[NFMAX/2+2],dev,x[NFMAX/2+2],y[NFMAX/2+2],alpha[NFMAX/2+2];
static double des[1045],grid[1045],wt[1045];

double dw(double x)
{
  return x/(2.0e00-x);
}



double ds(double x)
{
 return x/(1.0e00-0.5e00*x);
}



double d(int k, int n, int m)
{
  double q,qd;
  int l,j;

  qd=1.0e00;
  q=x[k];
  
  for (l=1; l<= m; l++) {
    j=l;
    do {
      if (j!=k) qd=2.0e00*qd*(q-x[j]);
      j+=m;
    } while (j<=n);
  }
  return (double) 1.0e00/qd;
}

double gee(int k, int n)
{
  double p,c,d,xf;
  int j;

  p=0.0e00;
  xf=grid[k];
  xf=cos(2*PI*xf);
  d=0.0e00;
  for (j=1; j<=n; j++) {
    c=xf-x[j];
    c=ad[j]/c;
    d=d+c;
    p=p+c*y[j];
  }
  return (double) p/d;
}


double eff(double temp, double *fx, double *wtx, int lband, int jtype)
{
  if (jtype!=3) return fx[lband];      /* multiband + hilbert */
  else return 2.0e00*fx[lband]*temp;     /*  differentiator */
}

double wate(double temp, double *fx, double *wtx, int lband, int jtype)
{
  double st;

  if (jtype==3)  {
    if (fx[lband]>=1.0e-04)  st=wtx[lband]/temp;
    else st=wtx[lband];
  } else
  st=wtx[lband];
  return st;
}

int grad(double dd, double ds, double df)
{
  double f,d;

  dd=log10(dd);
  ds=log10(ds);

  d=ds*(0.5309e-03*dd*dd+0.7114e-01*dd-0.4761e00)+(-0.266e-03*dd*dd-0.5941e00*dd-0.4278);
  f=1.1012e01+0.512*(dd-ds);
  return (int) floor((d/df)-f*df+0.5)+1;
}

/********************  REMEZ ALGORITHM FOR FIR FILTER ******************/
/** all variables correspond to the original program variables in the IEEE book on dsp **/

void fir1(double *edge, int nfilt, int nbands, int *error, double *fx, double *wtx, int jtype, double *h)

                    
                                
                
          
          

/* edge: array of normalized edge frequencies */
/* nfilt: number of taps */
/* nbands: number of bands */
/* error: error code returned by routine */
/* fx: array of desired frequency response in each band */
/* wtx: array of error weights for each band */
/* jtype: filter type (1=multiple pass/stopband, 2=differentiator, 3=hilbert xformer) */
/* h: 1-indexed array of filter coeff. (only the first half; other half is symmetric) */

{
  double delf,fup,temp,change;
  int    nfmax,anfilt,neg,nodd,lgrid;
  int    lband,nm1,nz;

  double a[NFMAX/2+2],p[NFMAX/2+1],q[NFMAX/2+1];  /* remez vars */
  int niter;
  int itrmax,nzz,jet,i,j,k,l,nu,jchnge;
  int k1,knz,klow,nut,nut1,kup,luck,kn,kkk;
  double devl,dtemp,dnum,dden,ynz,comp,y1;
  double err,gtemp,cn,aa,bb,xe,xt,fsh,ft;

  nfmax=NFMAX;

  if ((jtype==1) || (jtype>3)) {
    delf=0.5e00;i=2;
    do {
      temp=edge[i+1]-edge[i];
      if (temp<delf) {
        delf=temp;j=DIV(i,2);
      }
      i++;i++;
    } while (i!=2*nbands);
    if (wtx[j+1]>=wtx[j])
      anfilt=grad(wtx[j],wtx[j+1],delf);
    else
      anfilt=grad(wtx[j+1],wtx[j],delf);
  }

  if (nfilt==1) nfilt=anfilt;
  delf=0.0e00;j=1;
  FOR(i,1,nbands) {
    if (wtx[i]>=delf) delf=wtx[i];
  }
  FOR(i,1,nbands) wtx[i]=delf/wtx[i];
  if (nfilt>nfmax) { *error=3;exit(-1); }
  if ((jtype==5) || (jtype==7)) {
    if (MOD(nfilt,2)==0)  nfilt=nfilt+1; 
  }
  neg=1;

  lgrid=16;                                      /* grid density */

  if ((jtype==1) || (jtype>3)) neg=0;
  nodd=MOD(nfilt,2);nfcns=DIV(nfilt,2);
  if ((nodd==1)&&(neg==0)) nfcns++;
  grid[1]=edge[1];
  delf=0.5e00/INT(lgrid*nfcns);
  if (neg!=0) {
    if (edge[1]<delf)  grid[1]=delf;
  }
  j=1;l=1;lband=1;
  do {
    fup=edge[l+1];
    do {
      temp=grid[j];
      des[j]=eff(temp,fx,wtx,lband,jtype);
      wt[j]=wate(temp,fx,wtx,lband,jtype);
      j++;
      grid[j]=temp+delf;
    } while (grid[j]<=fup);
    grid[j-1]=fup;
    des[j-1]=eff(fup,fx,wtx,lband,jtype);
    wt[j-1]=wate(fup,fx,wtx,lband,jtype);
    lband++;
    l++;l++;
    if (lband<=nbands) grid[j]=edge[l];
  } while (lband<=nbands);
  ngrid=j-1;
  if (neg==nodd) {
    if (grid[ngrid]>(0.5e00-delf)) ngrid--;
  }

  if ((nodd==0)&&(neg<=0)) {
    FOR(j,1,ngrid) {
      change=cos(PI*grid[j]);
      des[j]=des[j]/change;
      wt[j]=wt[j]*change;
    }
  }
  if ((nodd==0)&&(neg>0)) {
    FOR(j,1,ngrid) {
      change=sin(PI*grid[j]);
      des[j]=des[j]/change;
      wt[j]=wt[j]*change;
    }
  }
  if ((nodd==1)&&(neg>0)) {
    FOR(j,1,ngrid) {
      change=sin(2*PI*grid[j]);
      des[j]=des[j]/change;
      wt[j]=wt[j]*change;
    }
  }
  temp=INT(ngrid-1)/INT(nfcns);
  FOR(j,1,nfcns) iext[j]=TRUNC(INT(j-1)*temp)+1;
  iext[nfcns+1]=ngrid;
  nm1=nfcns-1;nz=nfcns+1;

/*  remez; */

  itrmax=25;err=0.0e00;devl=-1.0e00;
  nz=nfcns+1;nzz=nfcns+2;niter=0;
 m100:
  iext[nzz]=ngrid+1;
  niter++;
/*  printf("Iteration #%d\n",niter); */
  if (niter>itrmax)  { *error=1; goto m400; }
  FOR(j,1,nz) x[j]=cos(2*PI*grid[iext[j]]);
  jet=DIV((nfcns-1),15)+1;
  FOR(j,1,nz) ad[j]=d(j,nz,jet);
  dnum=0.0e00;
  dden=0.0e00;
  k=1;
  FOR(j,1,nz) {
    l=iext[j];
    dnum=dnum+ad[j]*des[l];
    dden=dden+k*ad[j]/wt[l];
    k=-k;
  }
  dev=dnum/dden;
  nu=1;
  if (dev>0.0e00)  nu=-1;
  dev=-nu*dev;
  k=nu;
  FOR(j,1,nz) {
    l=iext[j];
    y[j]=des[l]+k*dev/wt[l];
    k=-k;
  }
  if (dev<devl)  { *error=2;goto m400; }
  devl=dev;jchnge=0;
  k1=iext[1];
  knz=iext[nz];
  klow=0;nut=-nu;
  j=1;

 m200:
  if (j==nzz)  ynz=comp;
  if (j>=nzz)  goto m300;
  kup=iext[j+1];
  l=iext[j]+1;nut=-nut;
  if (j==2)  y1=comp;comp=dev;
  if (l>=kup)  goto m220;
  err=(gee(l,nz)-des[l])*wt[l];
  if (nut*err<=comp) goto m220;
  comp=nut*err;
 m210:
  l++;
  if (l>=kup)  goto m215;
  err=(gee(l,nz)-des[l])*wt[l];
  if (nut*err<=comp) goto m215;
  comp=nut*err;
  goto m210;
 m215:
  iext[j]=l-1;
  j++; klow=l-1; jchnge++;
  goto m200;
 m220:
  l--;
 m225:
  l--;
  if (l<=klow) goto m250;
  err=(gee(l,nz)-des[l])*wt[l];
  if (nut*err>comp) goto m230;
  if (jchnge<=0) goto m225;
  goto m260;
 m230:
  comp=nut*err;
 m235:
  l--;
  if (l<=klow) goto m240;
  err=(gee(l,nz)-des[l])*wt[l];
  if (nut*err<=comp) goto m240;
  comp=nut*err;
  goto m235;
 m240:
  klow=iext[j];
  iext[j]=l+1;
  j++;jchnge++;
  goto m200;
 m250:
  l=iext[j]+1;
  if (jchnge>0) goto m215;
 m255:
  l++;
  if (l>=kup) goto m260;
  err=(gee(l,nz)-des[l])*wt[l];
  if (nut*err<=comp) goto m255;
  comp=nut*err;
  goto m210;
 m260:
  klow=iext[j];
  j++;
  goto m200;
 m300:
  if (j>nzz)  goto m320;
  if (k1>iext[1])  k1=iext[1];
  if (knz<iext[nz])  knz=iext[nz];
  nut1=nut;nut=-nu;l=0;kup=k1;


  comp=ynz*1.00001e00; 
/* important value, that iteration works, if necessary change this value */

  luck=1;
 m310:
  l++;
  if (l>=kup) goto m315;
  err=(gee(l,nz)-des[l])*wt[l];
  if (nut*err<=comp)  goto m310;
  comp=nut*err;
  j=nzz;
  goto m210;
 m315:
  luck=6;
  goto m325;
 m320:
  if (luck>9) goto m350;
  if (comp>y1) y1=comp;
  k1=iext[nzz];
 m325:
  l=ngrid+1;
  klow=knz;nut=-nut1;

  comp=y1*1.00001;
/* important value, that iteration works, if necessary change this value */
 
 m330:
  l--;
  if (l<=klow) goto m340;
  err=(gee(l,nz)-des[l])*wt[l];
  dtemp=nut*err-comp;
  if (dtemp<=0.0e00) goto m330;
  j=nzz;
  comp=nut*err;
  luck=luck+10;
  goto m235;
 m340:
  if (luck==6) goto m370;
  FOR(j,1,nfcns) iext[nzz-j]=iext[nz-j];
  iext[1]=k1;
  goto m100;
 m350:
  kn=iext[nzz];
  FOR(j,1,nfcns) iext[j]=iext[j+1];
  iext[nz]=kn;
  goto m100;
 m370:
  if (jchnge>0) goto m100;

  /* end of IDFT */

 m400:
  nm1=nfcns-1;
  fsh=1.00e-06;
  gtemp=grid[1];
  x[nzz]=-2.0e00;
  cn=2*nfcns-1;
  delf=1.0e00/cn;
  l=1;
  kkk=0;
  if ((edge[1]==0.0e00)&&(edge[2*nbands]==0.5e00))  kkk=1;
  if (nfcns<=3) kkk=1;
  if (kkk!=1) {
    dtemp=cos(2*PI*grid[1]);
    dnum=cos(2*PI*grid[ngrid]);
    aa=2.0e00/(dtemp-dnum);
    bb=-(dtemp+dnum)/(dtemp-dnum);
  }
  FOR(j,1,nfcns) {
    ft=INT(j-1)*delf;
    xt=cos(2*PI*ft);
    if (kkk!=1)  {
      xt=(xt-bb)/aa;
      ft=acos(xt)/(2.0*PI);
    }
  m410:
    xe=x[l];
    if (xt>xe) goto m420;
      if ((xe-xt)<fsh) goto m415;
      l++;
    goto m410;
  m415:
    a[j]=y[l];
    goto m425;
    m420:
    if ((xt-xe)<fsh) goto m415;
    grid[1]=ft;
    a[j]=gee(1,nz);
    m425:
    if (l>1) l--;
  }

/* a[i] are the values of G(jw), these will be transformed into the time domain with IDFT */

  dden=2.0*PI/cn;
  FOR(j,1,nfcns) {
    dtemp=0.0e00;
    dnum=(j-1)*dden;
    if (nm1>=1) {
      FOR(k,1,nm1) dtemp=dtemp+a[k+1]*cos(dnum*k);
    }
    dtemp=2.0e00*dtemp+a[1];
    alpha[j]=dtemp;
  }
  FOR(j,2,nfcns) alpha[j]=2.0e00*alpha[j]/cn;
  alpha[1]=alpha[1]/cn;
  if (kkk!=1) {
    p[1]=2.0e00*alpha[nfcns]*bb+alpha[nm1];
    p[2]=2.0e00*aa*alpha[nfcns];
    q[1]=alpha[nfcns-2]-alpha[nfcns];
    FOR(j,2,nm1) {
      if (j>=nm1) {
        aa=0.5e00*aa;
        bb=0.5e00*bb;
      }
      p[j+1]=0.0e00;
      FOR(k,1,j) {
        a[k]=p[k];
        p[k]=2.0e00*bb*a[k];
      }
      p[2]=p[2]+a[1]*2.0e00*aa;
      FOR(k,1,(j-1)) p[k]=p[k]+q[k]+aa*a[k+1];
      FOR(k,3,(j+1)) p[k]=p[k]+aa*a[k-1];
      if (j!=nm1) {
        FOR(k,1,j) q[k]=-a[k];
        q[1]=q[1]+alpha[nfcns-1-j];
      }
    }
    FOR(j,1,nfcns) alpha[j]=p[j];
  }
  if (nfcns<=3) {
    alpha[nfcns+1]=0.0e00;
    alpha[nfcns+2]=0.0e00;
  }


  if ((neg<=0)&&(nodd!=0)) {
    FOR(j,1,nm1) h[j]=0.5e00*alpha[nz-j];
    h[nfcns]=alpha[1];
  }
  if ((neg<=0)&&(nodd==0)) {
    h[1]=0.25e00*alpha[nfcns];
    FOR(j,2,nm1) h[j]=0.25e00*(alpha[nz-j]+alpha[nfcns+2-j]);
    h[nfcns]=0.5e00*alpha[1]+0.25e00*alpha[2];
  }
  if ((neg>0)&&(nodd!=0)) {
    h[1]=0.25e00*alpha[nfcns];
    h[2]=0.25e00*alpha[nm1];
    FOR(j,3,nm1) h[j]=0.25e00*(alpha[nz-j]-alpha[nfcns+3-j]);
    h[nfcns]=0.5e00*alpha[1]-0.25e00*alpha[3];
    h[nz]=0.0e00;nfcns=nfcns+1;
  }
  if ((neg>0)&&(nodd==0)) {
    h[1]=0.25e00*alpha[nfcns];
    FOR(j,2,nm1) h[j]=0.25e00*(alpha[nz-j]-alpha[nfcns+2-j]);
    h[nfcns]=0.5e00*alpha[1]-0.25e00*alpha[2];
  }
}


void design_bandpass_filter(double *h, double start, double end, int taps, double weight)
{
  double edge[7],fx[4],wtx[4];
  int bands,i,midpoint,error;

  edge[1]=0.0;
  if (start==0.0) { /*lopass*/
    edge[2]=(end-DELTA_F);
    edge[3]=(end+DELTA_F);
    edge[4]=0.5;
    fx[1]=1.0; fx[2]=0.0;
    wtx[1]=weight; wtx[2]=1.0;
    bands=2;
  } else if (end==0.5) {  /* highpass */
    edge[2]=(start-DELTA_F);
    edge[3]=(start+DELTA_F);
    edge[4]=end;
    fx[1]=0.0; fx[2]=1.0;
    wtx[1]=1.0; wtx[2]=weight;
    bands=2;
    if (!MOD(taps,2)) {
      printf("Warning: I'm designing a symmetric highpass filter with an even\n");
      printf("number of taps. I would suggest trying an odd number of taps for\n");
      printf("better results.\n");
    }
  } else {
    edge[2]=(start-DELTA_F);
    edge[3]=(start+DELTA_F);
    edge[4]=(end-DELTA_F);
    edge[5]=(end+DELTA_F);
    edge[6]=0.5;
    fx[1]=0.0; fx[2]=1.0; fx[3]=0.0;
    wtx[1]=1.0; wtx[2]=weight; wtx[3]=1.0;
    bands=3;
  }

  fir1(edge,taps,bands,&error,fx,wtx,1,h);
  
  /* tweak h to put it in a more useful form */
  if (MOD(taps,2)) {
    midpoint=DIV(taps,2)+1;
    for (i=0; i<midpoint; i++) h[i]=h[i+1];
    for (i=0; i<midpoint-1; i++)
      h[midpoint+i]=h[midpoint-2-i];
  }
  else {
    midpoint=DIV(taps,2);
    for (i=0; i<midpoint; i++) h[i]=h[i+1];
    for (i=0; i<midpoint; i++)
      h[midpoint+i]=h[midpoint-1-i];
  }
}


/* test code below */

/* main()

{
  double h[24],g[51];
  int i;

  design_bandpass_filter(h,0.0,0.16,24);

  printf("\n\n--------------------------\n");
  for(i=0;i<24;i++)
    printf("h[%d] = %lf\n",i,h[i]);

  design_bandpass_filter(h,0.16,0.32,51);

  printf("\n\n--------------------------\n");
  for(i=0;i<51;i++)
    printf("g[%d] = %lf\n",i,h[i]);
} */
