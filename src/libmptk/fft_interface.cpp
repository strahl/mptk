/******************************************************************************/
/*                                                                            */
/*                             fft_interface.cpp                              */
/*                                                                            */
/*                        Matching Pursuit Library                            */
/*                                                                            */
/* R�mi Gribonval                                                             */
/* Sacha Krstulovic                                           Mon Feb 21 2005 */
/* -------------------------------------------------------------------------- */
/*                                                                            */
/*  Copyright (C) 2005 IRISA                                                  */
/*                                                                            */
/*  This program is free software; you can redistribute it and/or             */
/*  modify it under the terms of the GNU General Public License               */
/*  as published by the Free Software Foundation; either version 2            */
/*  of the License, or (at your option) any later version.                    */
/*                                                                            */
/*  This program is distributed in the hope that it will be useful,           */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/*  GNU General Public License for more details.                              */
/*                                                                            */
/*  You should have received a copy of the GNU General Public License         */
/*  along with this program; if not, write to the Free Software               */
/*  Foundation, Inc., 59 Temple Place - Suite 330,                            */
/*  Boston, MA  02111-1307, USA.                                              */
/*                                                                            */
/******************************************************************************/
/*
 * SVN log:
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

/**************************************************************/
/*                                                            */
/* fft_interface.cpp: generic interface for FFT libraries     */
/*                                                            */
/**************************************************************/

#include "mptk.h"
#include "system.h"


/*********************************/
/*                               */
/* GENERIC INTERFACE             */
/*                               */
/*********************************/


/***************************/
/* CONSTRUCTORS/DESTRUCTOR */
/***************************/

/***********************************/
/* Constructor with a typed window */
MP_FFT_Generic_Interface_c::MP_FFT_Generic_Interface_c( const unsigned long int setWindowSize,
							const unsigned char setWindowType,
							const double setWindowOption,
							const unsigned long int setFftRealSize ) {
  extern MP_Win_Server_c MP_GLOBAL_WIN_SERVER;

  /* Check if fftCplxSize will overflow in the expression fftCplxSize = 2*(fftRealSize-1 */
  assert( setFftRealSize    <= ULONG_MAX>>1 );

  /* Set values */
  windowSize = setWindowSize;
  windowType = setWindowType;
  windowOption = setWindowOption;
  fftRealSize = setFftRealSize;
  fftCplxSize = (fftRealSize-1)<<1;
  assert( fftCplxSize >= setWindowSize );

  /* Compute the window and get its center point */
  windowCenter = MP_GLOBAL_WIN_SERVER.get_window( &window, windowSize, windowType, windowOption );

  /* Allocate the atom's autocorrelations */
  if ( alloc_correl() ) {
    fprintf( stderr, "mplib warning -- MP_FFT_Generic_Interface_c() - "
	     "The allocation of the atom's autocorrelations returned an error.\n");
  }

}


/**************/
/* Destructor */
MP_FFT_Generic_Interface_c::~MP_FFT_Generic_Interface_c( ) {

  if ( reCorrel  ) free( reCorrel  );
  if ( imCorrel  ) free( imCorrel  );
  if ( sqCorrel  ) free( sqCorrel  );
  if ( cstCorrel ) free( cstCorrel );

}



/***************************/
/* OTHER METHODS           */
/***************************/

/*****************************************/
/* Allocation of the correlation vectors */
int MP_FFT_Generic_Interface_c::alloc_correl( void ) {
 
  /* Allocate the memory for the correlations and init it to zero */
  reCorrel = imCorrel = sqCorrel = NULL;
  /* Reminder: ( fftCplxSize == ((fftRealSize-1)<<1) ) <=> ( fftRealSize == ((fftCplxSize>>1)+1) ) */
  if ( ( reCorrel = (MP_Real_t *) calloc( fftRealSize , sizeof(MP_Real_t)) ) == NULL) {
    fprintf( stderr, "mplib warning -- alloc_correl() - Can't allocate storage space for the real part"
	     " of the atom correlations. Correlations are left un-initialized.\n");
    return( 1 );
  }
  else if ( ( imCorrel = (MP_Real_t *) calloc( fftRealSize , sizeof(MP_Real_t)) ) == NULL) {
    fprintf( stderr, "mplib warning -- alloc_correl() - Can't allocate storage space for the imaginary part"
	     " of the atom correlations. Correlations are left un-initialized.\n");
    free(reCorrel); reCorrel = NULL;
    return( 1 );
  }
  else if ( ( sqCorrel = (MP_Real_t *) calloc( fftRealSize , sizeof(MP_Real_t)) ) == NULL) {
    fprintf( stderr, "mplib warning -- alloc_correl() - Can't allocate storage space for the squared"
	     " atom correlations. Correlations are left un-initialized.\n");
    free(reCorrel); reCorrel = NULL;
    free(imCorrel); imCorrel = NULL;
    return( 1 );
  }
  else if ( ( cstCorrel = (MP_Real_t *) calloc( fftRealSize , sizeof(MP_Real_t)) ) == NULL) {
    fprintf( stderr, "mplib warning -- alloc_correl() - Can't allocate storage space for the pre-computed"
	     " constant of the atom correlations. Correlations are left un-initialized.\n");
    free(reCorrel); reCorrel = NULL;
    free(imCorrel); imCorrel = NULL;
    free(sqCorrel); sqCorrel = NULL;
    return( 1 );
  }

  return( 0 );
}



/*********************************/
/*                               */
/* FFTW-DEPENDENT IMPLEMENTATION */
/*                               */
/*********************************/

/***************************/
/* CONSTRUCTORS/DESTRUCTOR */
/***************************/

/* Code factorisation for FFTW constructors */
inline void MP_FFTW_Interface_c::common_FFTW_constructor( void ) {

  /* FFTW takes integer FFT sizes => check if the cast (int)(fftCplxSize) will overflow. */
  assert( fftCplxSize <= INT_MAX );

  /* Allocate the necessary buffers */
  inPrepared =       (double*) fftw_malloc( sizeof(double)       * fftCplxSize );
  out        = (fftw_complex*) fftw_malloc( sizeof(fftw_complex) * fftRealSize );
  /* Call the FFTW planning utility */
  p = fftw_plan_dft_r2c_1d( (int)(fftCplxSize), inPrepared, out, FFTW_MEASURE );
  
  /* Tabulate the atom's autocorrelations */
  if ( fill_correl() ) {
    fprintf( stderr, "mplib warning -- common_FFTW_constructor() - "
	     "The allocation of the atom's autocorrelations returned an error.\n" );
  }
  
  return;
}

/****/
/* Constructor where the window is actually generated */
MP_FFTW_Interface_c::MP_FFTW_Interface_c( const unsigned long int setWindowSize,
					  const unsigned char setWindowType,
					  const double setWindowOption,
					  const unsigned long int setFftRealSize )
  :MP_FFT_Generic_Interface_c( setWindowSize, setWindowType, setWindowOption, setFftRealSize ) {

  common_FFTW_constructor();
}


/**************/
/* Destructor */
MP_FFTW_Interface_c::~MP_FFTW_Interface_c() {

  fftw_free( inPrepared );
  fftw_free( out );
  fftw_destroy_plan( p );
}


/***************************/
/* OTHER METHODS           */
/***************************/

/******************************************************/
/** Fill the correlation arrays with 
 * \f$ (\mbox{reCorrel}[k],\mbox{imCorrel[k]}) =
 * \sum_{n=0}^{fftCplxSize-1} \mbox{window}^2[n] e^{2i\pi \frac{2kn}{fftCplxSize}} \f$ */
int MP_FFTW_Interface_c::fill_correl( void ) {

  MP_Sample_t buffer[windowSize];
  double re,im,sq;
  int k, cursor;

  /* 1/ Cast the window: */
  for ( k = 0; k < (int)(windowSize); k++ ) buffer[k] = (MP_Sample_t)( window[k] );
  /* Note: the window will be multiplied by itself and padded in exec(). */

  /* 2/ Compute its FFT: */
  exec( buffer );

  /* 3/ Fill reCorrel and imCorrel with the adequate FFT values: */
  for ( k = cursor = 0;  cursor < (int)(fftRealSize);  k++, cursor += 2 ) {
    /* In this loop, cursor is always equal to 2*k. */
    re = out[cursor][0];
    im = out[cursor][1];
    *( reCorrel + k ) = (MP_Real_t)(   re );
    *( imCorrel + k ) = (MP_Real_t)( - im );
    sq = ( re*re + im*im );
    *( sqCorrel + k ) = (MP_Real_t)(   sq );
    *( cstCorrel + k ) = (MP_Real_t)( 2.0 / (1.0 - sq) );
    /* Rectify a possible numerical innacuracy at DC frequency: */
    if ( k==0 ) {
      *( reCorrel + k )  = 1.0;
      *( imCorrel + k )  = 0.0;
      *( sqCorrel + k )  = 1.0;
      *( cstCorrel + k ) = 1.0;
    }
    else {
      if ( (MP_Real_t)(sq) >= 1.0 ) {
	fprintf( stderr, "mplib warning -- fill_correl() - atom's autocorrelation has value >= 1.0 [diff= %e ]\n"
		 "\t\tfor frequency index %d (fftRealSize in this block is %lu).\n",
		 ((MP_Real_t)(sq) - 1.0), k, fftRealSize );
      }
    }
  }
  for ( cursor = (fftCplxSize-cursor);  cursor >= 0 ;  k++, cursor -= 2 ) {
    /* In this loop, cursor is always equal to (fftCplxSize - 2*k). */
    re = out[cursor][0];
    im = out[cursor][1];
    *( reCorrel + k ) = (MP_Real_t)( re );
    *( imCorrel + k ) = (MP_Real_t)( im );
    sq = ( re*re + im*im );
    *( sqCorrel + k ) = (MP_Real_t)( sq );
    *( cstCorrel + k ) = (MP_Real_t)( 2.0 / (1.0 - sq) );
    /* Rectify a possible numerical innacuracy at Nyquist frequency: */
    if ( k == ((int)(fftRealSize)-1) ) {
      *( reCorrel + k )  = 1.0;
      *( imCorrel + k )  = 0.0;
      *( sqCorrel + k )  = 1.0;
      *( cstCorrel + k ) = 1.0;
    }
    else {
      if ( (MP_Real_t)(sq) >= 1.0 ) {
	fprintf( stderr, "mplib warning -- fill_correl() - atom's autocorrelation has value >= 1.0 [diff= %e ]\n"
		 "\t\tfor frequency index %d (fftRealSize in this block is %lu).\n",
		 ((MP_Real_t)(sq) - 1.0), k, fftRealSize );
      }
    }
  }

  return( 0 );
}


/*****************************************/
/* Apply the window and execute the plan */
inline void MP_FFTW_Interface_c::exec( MP_Sample_t *in ) {

  unsigned long int i;

  /* Did anyone hook some buffers ? */
  assert( in  != NULL );
  assert( window  != NULL );

  /* Copy and window the input signal */
  for ( i=0; i<windowSize; i++ ) {
    *(inPrepared+i) = (double)(*(window+i)) * (double)(*(in+i));
  }
  /* Perform the zero padding */
  for ( i=windowSize; i<fftCplxSize; i++ ) {
    *(inPrepared+i) = 0.0;
  }

  /* Execute the FFT described by plan "p"
     (which itself points to the right input/ouput buffers,
     such as buffer inPrepared etc.) */
  fftw_execute( p );

}


/**************************/
/* Get the complex result */
void MP_FFTW_Interface_c::exec_complex( MP_Sample_t *in, MP_Real_t *re, MP_Real_t *im ) {

  unsigned long int i;
  double re_out, im_out;

  /* Simple buffer check */
  assert( in != NULL );
  assert( re != NULL );
  assert( im != NULL );

  /* Execute the FFT */
  exec( in );

  /* Cast and copy the result */
  for ( i=0; i<fftRealSize; i++ ) {
    re_out = out[i][0];
    im_out = out[i][1];
    *(re+i) = (MP_Real_t)( re_out );
    *(im+i) = (MP_Real_t)( im_out );
  }
  /* Ensure that the imaginary part of the DC and Nyquist frequency components are zero */
  *(im) = 0.0;
  *(im+fftRealSize-1) = 0.0;

}

/**************************/
/* Get the magnitude only */
void MP_FFTW_Interface_c::exec_mag( MP_Sample_t *in, MP_Real_t *mag ) {

  unsigned long int i;
  double re, im;

  /* Simple buffer check */
  assert( in  != NULL );
  assert( mag != NULL );

  /* Execute the FFT */
  exec( in );

  /* Get the resulting magnitudes */
  for ( i=0; i<fftRealSize; i++ ) {
    re = out[i][0];
    im = out[i][1];

#ifdef MP_MAGNITUDE_IS_SQUARED
    *(mag+i) = (MP_Real_t)( re*re+im*im );
#else
    *(mag+i) = (MP_Real_t)( sqrt( re*re+im*im ) );
#endif
  }

}


/***********************/
/* Get the energy only */
void MP_FFTW_Interface_c::exec_energy( MP_Sample_t *in, MP_Real_t *mag ) {

  int i;
  double re, im, reSq, imSq, energy;
  double correlSq;

  /* Simple buffer check */
  assert( in  != NULL );
  assert( mag != NULL );

  /* Execute the FFT */
  exec( in );

  /*****/
  /* Get the resulting magnitudes: */

  /* -- At frequency 0: */
  re = out[0][0];
  *(mag) = (MP_Real_t)( re * re );

  /* -- At a frequency between 0 and Nyquist: */
  for ( i = 1;  i < ((int)(fftRealSize) - 1);  i++ ) {

    /* Get the complex values */
    re = out[i][0];
    im = out[i][1];
    reSq = ( re * re );
    imSq = ( im * im );

    /* Get the atom' autocorrelation: */
    correlSq = (double)(*(sqCorrel+i));

    /* If the atom's autocorrelation is neglegible: */
    if ( correlSq < MP_ENERGY_EPSILON ) {
      energy = 2 * ( reSq + imSq );
    }
    /* Else, if the atom's autocorrelation is NOT neglegible: */
    else {
	energy  =   ( reSq + imSq )
	          - (double)(*(reCorrel+i)) * ( reSq - imSq )
	          + (double)(*(imCorrel+i)) * (  2 * re*im  );
      
	energy = (double)(*(cstCorrel+i)) * energy;
	/* The following version appears to be slightly slower,
	   but that's not clear cut with our experiments: */
	/* energy = ( 2.0 / (1.0 - correlSq) ) * energy; */
      }

    /* => Compensate for a possible numerical innacuracy
     *    (this case should never happen in practice) */
    if ( energy < 0 ) {
      fprintf( stderr, "mplib warning -- exec_energy() - A negative energy was met."
	       " (energy = [%g])\nEnergy value is reset to 0.0 .", energy );
      energy = 0.0;
    }

    /* Cast and fill mag */
    *(mag+i) = (MP_Real_t)(energy);

  }

  /* -- At the Nyquist frequency: */
  re = out[fftRealSize-1][0];
  *(mag+fftRealSize-1) = (MP_Real_t)( re * re );

  /*****/

  return;
}
