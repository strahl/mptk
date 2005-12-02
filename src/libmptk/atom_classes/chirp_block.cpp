/******************************************************************************/
/*                                                                            */
/*                             chirp_block.cpp                                */
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
 * $Author: sacha $
 * $Date: 2005-07-25 20:37:06 +0200 (Mon, 25 Jul 2005) $
 * $Revision: 22 $
 *
 */

/***************************************************/
/*                                                 */
/* chirp_block.cpp: methods for chirp blocks       */
/*                                                 */
/***************************************************/

#include "mptk.h"
#include "mp_system.h"


/***************************/
/* CONSTRUCTORS/DESTRUCTOR */
/***************************/

/************************/
/* Specific constructor */
MP_Chirp_Block_c::MP_Chirp_Block_c( MP_Signal_c *setSignal,
				    const unsigned long int setFilterLen,
				    const unsigned long int setFilterShift,
				    const unsigned long int setFftRealSize,
				    const unsigned char setWindowType,
				    const double setWindowOption,
				    const unsigned int setNumFitPoints,
				    const unsigned int setNumIter )
:MP_Gabor_Block_c( setSignal, setFilterLen, setFilterShift, 
		   setFftRealSize, setWindowType, setWindowOption ) {

  numFitPoints = setNumFitPoints;
  totNumFitPoints = 2*numFitPoints+1;
  numIter = setNumIter;

  /* Allocate all the buffers */

  /* - Demodulation chirp signal: */
  if ( (chirpRe = (MP_Real_t*) malloc(filterLen*sizeof(MP_Real_t)) ) == NULL ) {
    fprintf( stderr, "mptk warning -- MP_Chirp_Block_c() - Can't allocate an array of [%lu] MP_Real_t elements"
	     " for the chirpRe array. This pointer will remain NULL.\n", filterLen );
  } 
  if ( (chirpIm = (MP_Real_t*) malloc(filterLen*sizeof(MP_Real_t)) ) == NULL ) {
    fprintf( stderr, "mptk warning -- MP_Chirp_Block_c() - Can't allocate an array of [%lu] MP_Real_t elements"
	     " for the chirpIm array. This pointer will remain NULL.\n", filterLen );
  } 

  /* - Input signal x demodulation chirp: */
  if ( (sigChirpRe = (MP_Real_t*) malloc(filterLen*sizeof(MP_Real_t)) ) == NULL ) {
    fprintf( stderr, "mptk warning -- MP_Chirp_Block_c() - Can't allocate an array of [%lu] MP_Real_t elements"
	     " for the sigChirpRe array. This pointer will remain NULL.\n", filterLen );
  } 
  if ( (sigChirpIm = (MP_Real_t*) malloc(filterLen*sizeof(MP_Real_t)) ) == NULL ) {
    fprintf( stderr, "mptk warning -- MP_Chirp_Block_c() - Can't allocate an array of [%lu] MP_Real_t elements"
	     " for the sigChirpIm array. This pointer will remain NULL.\n", filterLen );
  } 

  /* - Misc: */
  if ( (fftEnergy = (MP_Real_t*) malloc(fftRealSize*sizeof(MP_Real_t)) ) == NULL ) {
    fprintf( stderr, "mptk warning -- MP_Chirp_Block_c() - Can't allocate an array of [%lu] MP_Real_t elements"
	     " for the fftEnergy array. This pointer will remain NULL.\n", fftRealSize );
  } 

  if ( (logAmp = (MP_Real_t*) malloc((totNumFitPoints)*sizeof(MP_Real_t)) ) == NULL ) {
    fprintf( stderr, "mptk warning -- MP_Chirp_Block_c() - Can't allocate an array of [%u] MP_Real_t elements"
	     " for the logAmp array. This pointer will remain NULL.\n", totNumFitPoints );
  } 
  if ( (phase = (MP_Real_t*) malloc((totNumFitPoints)*sizeof(MP_Real_t)) ) == NULL ) {
    fprintf( stderr, "mptk warning -- MP_Chirp_Block_c() - Can't allocate an array of [%u] MP_Real_t elements"
	     " for the phase array. This pointer will remain NULL.\n", totNumFitPoints );
  } 

  if ( alloc_correl( &reCorrelChirp, &imCorrelChirp, &sqCorrelChirp, &cstCorrelChirp ) ) {
    fprintf( stderr, "mplib warning -- MP_FFT_Generic_Interface_c() - "
	     "The allocation of the atom's autocorrelations returned an error.\n");
  }

}


/**************/
/* Destructor */
MP_Chirp_Block_c::~MP_Chirp_Block_c() {

#ifndef NDEBUG
  fprintf( stderr, "libmptk DEBUG -- ~MP_Chirp_Block_c() - Deleting chirp_block.\n" );
#endif

  if ( chirpRe ) free(chirpRe);
  if ( chirpIm ) free(chirpIm);

  if ( sigChirpRe ) free(sigChirpRe);
  if ( sigChirpIm ) free(sigChirpIm);
  if ( fftEnergy ) free(fftEnergy);

  if ( logAmp ) free(logAmp);
  if ( phase  ) free(phase);

  if ( reCorrelChirp  ) free( reCorrelChirp  );
  if ( imCorrelChirp  ) free( imCorrelChirp  );
  if ( sqCorrelChirp  ) free( sqCorrelChirp  );
  if ( cstCorrelChirp ) free( cstCorrelChirp );
}


/***************************/
/* OTHER METHODS           */
/***************************/

/********/
/* Type */
char* MP_Chirp_Block_c::type_name() {
  return ("chirp");
}


/********/
/* Readable text dump */
int MP_Chirp_Block_c::info( FILE* fid ) {

  int nChar = 0;

  nChar += fprintf( fid, "libmptk info -- CHIRP BLOCK: %s window (window opt=%g)", window_name( fft->windowType ), fft->windowOption );
  nChar += fprintf( fid, " of length [%lu], shifted by [%lu] samples, projected on [%lu] frequencies;",
		    filterLen, filterShift, numFilters );
  nChar += fprintf( fid, " numFitPoints is [%u], numIter is [%u];\n", numFitPoints, numIter );
  nChar += fprintf( fid, "libmptk info -- The number of frames for this block is [%lu], the search tree has [%lu] levels.\n",
		    numFrames, numLevels );
  return ( nChar );
}


/***************************************/
/* Output of the ith atom of the block */
unsigned int MP_Chirp_Block_c::create_atom( MP_Atom_c **atom,
					    const unsigned long int atomIdx ) {
  MP_Gabor_Atom_c *gatom = NULL;
  unsigned long int freqIdx, k;
  MP_Real_t chirprate;
  unsigned int l;
  unsigned int iter;

  int chanIdx, numChans;
  MP_Sample_t *in;

  MP_Real_t reCenter,imCenter,sqCenter;
  MP_Real_t re,im,reSq,imSq,energy,real,imag,amp,atomphase;
  MP_Real_t a,b,d,e,fftCplxSize;
  MP_Real_t lambda,mu,deltaChirp;
  //MP_Real_t alpha,beta;

#ifndef NDEBUG
  fprintf( stderr, "mptk DEBUG -- Entering CHIRP::create_atom.\n" ); fflush( stderr );
#endif

  /* Useful dereferences */
  numChans = s->numChans;
  fftCplxSize = (MP_Real_t)(fft->fftCplxSize);

  /* Create the best Gabor atom with chirprate zero */
  if ( ( MP_Gabor_Block_c::create_atom( atom, atomIdx ) ) == 0 ) {
    fprintf( stderr, "mptk error -- MP_Chirp_Block_c::create_atom() -"
	     " Can't create a new Gabor atom in create_atom()."
	     " Returning NULL as the atom reference.\n" );
    return( 0 );
  }
  gatom = (MP_Gabor_Atom_c*) (*atom);
  chirprate = gatom->chirp;

#ifndef NDEBUG
  gatom->info( stderr ); fflush( stderr );
#endif

  /******************/
  /* I) ITERATION 0 */
  /******************/
  /* Note: during this iteration, the chirprate is equal to 0. */

  /*****************************/
  /* I.1) FIT A NEW CHIRP RATE */

  /* Find the index closest to the frequency of the current atom */
  freqIdx = (unsigned long int) round( (double)(gatom->freq) * (double)(fftCplxSize) );

#ifndef NDEBUG
  fprintf( stderr, "mptk DEBUG -- freqIdx was = %lu , freq = %f (cplxSize = %g).\n",
	   freqIdx, gatom->freq, fftCplxSize ); fflush( stderr );
#endif

  /* If there is not enough fit points on both sides of the frequency, 
   * keep the current chirprate ( =0 ) and return the unchanged gabor atom. */
  if ( (freqIdx <= (unsigned long int)numFitPoints) ||
       ( (freqIdx+(unsigned long int)numFitPoints) >= fftRealSize ) ) {
#ifndef NDEBUG
    fprintf( stderr, "mptk DEBUG -- freqIdx = %lu , RETURNING.\n", freqIdx ); fflush( stderr );
#endif
    return( 1 );
  }

  /* Reset the logamp and phase accumulators */
  for ( l = 0; l < totNumFitPoints; l++ ) logAmp[l] = phase[l] = 0.0;

  /* Compute an FFT per channel and fill the buffers of points to fit */
  for ( chanIdx = 0; chanIdx < numChans; chanIdx++ ) {

    assert ( s->channel[chanIdx] != NULL );

    /* Re-compute the complex FFT */
    in = s->channel[chanIdx] + gatom->support[chanIdx].pos;
    fft->exec_complex( in , fftRe, fftIm );

    /* Normalize the FFT at the center point */
    reCenter = fftRe[freqIdx];
    imCenter = fftIm[freqIdx]; 
    sqCenter = reCenter*reCenter + imCenter*imCenter;
    reCenter = reCenter/sqCenter;
    imCenter = - imCenter/sqCenter;

    /* Convert to 'logpolar' coordinates after division by the complex value at the center point,
       and accumulate the logAmp and phase value for the cross-channel average */
    for ( l = 0; l < totNumFitPoints; l++ ) {
      re = fftRe[freqIdx-numFitPoints+l]*reCenter - fftIm[freqIdx-numFitPoints+l]*imCenter;
      im = fftRe[freqIdx-numFitPoints+l]*imCenter + fftIm[freqIdx-numFitPoints+l]*reCenter;
      logAmp[l] += (MP_Real_t)( log( fabs(re*re+im*im) ) ); /* Division by two (corresponding to a square root)
							       is done when averaging over channels */
      phase[l]  += (MP_Real_t)( atan2(im,re) );
    }  /* <-- end loop on fit points */

  }   /* <-- end loop on channels */

  /* Finalize the average logAmp / phase over channels */
  for ( l = 0; l < totNumFitPoints; l++ ) {
    logAmp[l] = logAmp[l] / (2*numChans);
    phase[l]  = phase[l]  / numChans;
  }   /* <-- end loop on fit points */
  
  /* Perform the regression on the fit points */
  parabolic_regression( logAmp, phase, numFitPoints,
		        &a, &b, &d, &e );

  /* Convert the result into a new chirprate */
  lambda = - ( fftCplxSize * fftCplxSize ) * a * MP_INV_PI_SQ;
  mu     = - ( fftCplxSize * fftCplxSize ) * d * MP_INV_PI_SQ;
  //alpha  = ( freqIdx - b/(2*a) ) / fftCplxSize;
  //beta   = ( freqIdx - e/(2*d) ) / fftCplxSize;

  deltaChirp = mu/(MP_PI*(lambda*lambda+mu*mu));
  chirprate += deltaChirp;

#ifndef NDEBUG
  fprintf( stderr, "mptk DEBUG -- iter  0 : delta = %g , new chirp = %g.\n",
	   deltaChirp, chirprate ); fflush( stderr );
#endif

  /**********************************************/
  /* I.2) RE-LOCATE THE ATOM'S CENTER FREQUENCY */

  /* Update the chirp demodulation signal and the related correlations */
  set_chirp_demodulator( chirprate );

  /* Compute the new inner products with the complex chirp on each channel
     and update the energy over all channels at each frequency */
  for ( k = 0; k < fftRealSize; k++ ) {
    fftEnergy[k] = 0.0;
  }
    
  for ( chanIdx = 0; chanIdx < numChans; chanIdx++ ) {
      
    /* Compute FFT(sig*demodulator) */
    in = s->channel[chanIdx] + gatom->support[chanIdx].pos;
    fft->exec_complex_demod( in, chirpRe, chirpIm, fftRe, fftIm );

    /* Compute the magnitude of the best real chirp atom for each frequency */
    for ( k = 0;  k < fftRealSize;  k++ ) {
      
      /* Get the complex values */
      re = fftRe[k];
      im = fftIm[k];
      reSq = ( re * re );
      imSq = ( im * im );
      /* If the atom's autocorrelation is neglegible: */
	if ( sqCorrelChirp[k] < MP_ENERGY_EPSILON ) {
	  energy = 2 * ( reSq + imSq );
	}
	/* Else, if the atom's autocorrelation is NOT neglegible: */
	else {
	  energy  =   ( reSq + imSq )
	    - reCorrelChirp[k] * ( reSq - imSq )
	    + imCorrelChirp[k] * (  2 * re*im  );
	  energy = cstCorrelChirp[k] * energy;
	}
	/* => Compensate for a possible numerical innacuracy
	 *    (this case should never happen in practice) */
	if ( energy < 0 ) {
	  fprintf( stderr, "mptk warning -- MP_Chirp_Block_c::create_atom() - A negative energy was met."
		   " (energy = [%g])\nEnergy value is reset to 0.0 .", energy );
	  energy = 0.0;
	}
	
	/* Cast and fill mag */
	fftEnergy[k] += energy;
      }    
    } /* <-- end loop on channels */
  
    
    /* Find the best frequency */
    energy = 0.0;
    for ( k = 0; k < fftRealSize; k++ ) {
      if ( fftEnergy[k] > energy) { energy = fftEnergy[k]; freqIdx = k; }
    }

#ifndef NDEBUG
    fprintf( stderr, "mptk DEBUG -- iter  0 : New freqIdx = %lu.\n", freqIdx ); fflush( stderr );
#endif

    /****************************/
    /* II) FOLLOWING ITERATIONS */
    /****************************/

    for ( iter = 1; iter < numIter; iter++ ) {

      /******************************/
      /* II.1) FIT A NEW CHIRP RATE */

      /* If there is not enough fit points on both sides of the frequency, 
       * keep the current chirprate (=0) and stop */
      if ( (freqIdx <= (unsigned long int)numFitPoints) ||
	   ( (freqIdx+(unsigned long int)numFitPoints) >= fftRealSize ) ) break;

      /* Reset the logamp and phase accumulators */
      for ( l = 0; l < totNumFitPoints; l++ ) logAmp[l] = phase[l] = 0.0;

      /* Compute an FFT per channel and fill the buffers of points to fit */
      for ( chanIdx = 0; chanIdx < numChans; chanIdx++ ) {
      
	/* Compute the FFT of the demodulated signal */
	in = s->channel[chanIdx] + gatom->support[chanIdx].pos;
	fft->exec_complex_demod( in, chirpRe, chirpIm, fftRe, fftIm );

	/* Normalize the FFT at the center point */
	reCenter = fftRe[freqIdx];
	imCenter = fftIm[freqIdx]; 
	sqCenter = reCenter*reCenter + imCenter*imCenter;
	reCenter = reCenter/sqCenter;
	imCenter = - imCenter/sqCenter;
      
	/* Convert to 'logpolar' coordinates after division by the value at the center point,
	   and accumulate the logAmp and phase value for the cross-channel average */
	for ( l = 0; l < totNumFitPoints; l++ ) {
	  re = fftRe[freqIdx-numFitPoints+l]*reCenter - fftIm[freqIdx-numFitPoints+l]*imCenter;
	  im = fftRe[freqIdx-numFitPoints+l]*imCenter + fftIm[freqIdx-numFitPoints+l]*reCenter;
	  logAmp[l] += (MP_Real_t)( log( fabs(re*re+im*im) ) ); /* Division by two (corresponding to a square root)
								   is done when averaging over channels */
	  phase[l]  += (MP_Real_t)( atan2(im,re) );
	}  /* <-- end loop on fit points */      
      
      } /* <-- end loop on channels */
    
      /* Finalize the average logAmp / phase over channels */
      for ( l = 0; l < totNumFitPoints; l++ ) {
	logAmp[l] = logAmp[l] / (2*numChans);
	phase[l]  = phase[l]  / numChans;
      }   /* <-- end loop on fit points */
    
      /* Perform the regression on the fit points */
      parabolic_regression( logAmp, phase, numFitPoints,
			    &a, &b, &d, &e );
    

      /* Convert the result into a new chirprate */
      lambda = - ( fftCplxSize * fftCplxSize ) * a * MP_INV_PI_SQ;
      mu     = - ( fftCplxSize * fftCplxSize ) * d * MP_INV_PI_SQ;
      //alpha  = ( freqIdx - b/(2*a) ) / fftCplxSize;
      //beta   = ( freqIdx - e/(2*d) ) / fftCplxSize;
    
      deltaChirp = mu/(MP_PI*(lambda*lambda+mu*mu));
      chirprate += deltaChirp;

#ifndef NDEBUG
  fprintf( stderr, "mptk DEBUG -- iter %2d : delta = %g , new chirp = %g.\n",
	   iter, deltaChirp, chirprate ); fflush( stderr );
#endif

      /***********************************************/
      /* II.2) RE-LOCATE THE ATOM'S CENTER FREQUENCY */

      /* Update (chirpRe,chirpIm), the real and imaginary parts of exp(-i*pi*chirprate*t^2) 
       * as well as the correlations reCorrelChirp, imCorrelChirp, sqCorrelChirp, cstCorrelChirp between complex chirp 
       * atoms and their conjugates */
      set_chirp_demodulator( chirprate );
    
      /* Compute the new inner products with the complex chirp on each channel
	 and update the energy over all channels at each frequency */
      for ( k = 0; k < fftRealSize; k++ ) fftEnergy[k] = 0.0;
    
      for ( chanIdx = 0; chanIdx < numChans; chanIdx++ ) {
      
	/* Compute the FFT of the demodulated signal */
	in = s->channel[chanIdx] + gatom->support[chanIdx].pos;
	fft->exec_complex_demod( in, chirpRe, chirpIm, fftRe, fftIm );

	/* Compute the magnitude of the best real chirp atom for each frequency */
	for ( k = 0;  k < fftRealSize;  k++ ) {
	
	  /* Get the complex values */
	  re = fftRe[k];
	  im = fftIm[k];
	  reSq = ( re * re );
	  imSq = ( im * im );
	  /* If the atom's autocorrelation is neglegible: */
	  if ( sqCorrelChirp[k] < MP_ENERGY_EPSILON ) {
	    energy = 2 * ( reSq + imSq );
	  }
	  /* Else, if the atom's autocorrelation is NOT neglegible: */
	  else {
	    energy  =   ( reSq + imSq )
	      - reCorrelChirp[k] * ( reSq - imSq )
	      + imCorrelChirp[k] * (  2 * re*im  );
	    energy = cstCorrelChirp[k] * energy;
	  }
	  /* => Compensate for a possible numerical innacuracy
	   *    (this case should never happen in practice) */
	  if ( energy < 0 ) {
	    fprintf( stderr, "mptk warning -- MP_Chirp_Block_c::create_atom() -"
		     " A negative energy was met."
		     " (energy = [%g])\nEnergy value is reset to 0.0 .", energy );
	    energy = 0.0;
	  }
	
	  /* Cast and fill mag */
	  fftEnergy[k] += energy;
	}    
      } /* <-- end loop on channels */
  
    
      /* Find the best frequency */
      energy = 0.0;
      for ( k = 0; k < fftRealSize; k++ ) {
	if ( fftEnergy[k] > energy) { energy = fftEnergy[k]; freqIdx = k; }
      }

#ifndef NDEBUG
    fprintf( stderr, "mptk DEBUG -- iter %2d : New freqIdx = %lu.\n", iter, freqIdx ); fflush( stderr );
#endif

    } /* end loop on iterations */
    

    /******************************************************/
    /* III) ESTIMATE ALL THE PARAMETERS OF THE FINAL ATOM */
    /******************************************************/

    gatom->chirp = chirprate;
    gatom->freq  = (double)(freqIdx) / (double)(fftCplxSize);

#ifndef NDEBUG
    fprintf( stderr, "mptk DEBUG -- freqIdx is now = %lu , freq = %f (cplxSize = %g).\n",
	     freqIdx, gatom->freq, fftCplxSize ); fflush( stderr );
#endif

    /* Compute the magnitude of the best real chirp atom for each frequency and each channel */
    for ( chanIdx = 0; chanIdx < numChans; chanIdx++ ) {
      
      /* Compute the FFT of the demodulated signal */
      in = s->channel[chanIdx] + gatom->support[chanIdx].pos;
      fft->exec_complex_demod( in, chirpRe, chirpIm, fftRe, fftIm );

      re  = (double)( fftRe[freqIdx] ); 
      im  = (double)( fftIm[freqIdx] );
      energy = re*re + im*im;
#ifndef NDEBUG
      assert( sqCorrelChirp[freqIdx] <= 1.0 );
#endif
      /* Cf. explanations about complex2amp_and_phase() in general.h */
      if ( (freqIdx != 0) && ( (freqIdx+1) < fftRealSize ) ) {  
	real = (1.0 - reCorrelChirp[freqIdx])*re + imCorrelChirp[freqIdx]*im;
	imag = (1.0 + reCorrelChirp[freqIdx])*im + imCorrelChirp[freqIdx]*re;
	amp   = 2.0 * sqrt( real*real + imag*imag );
	atomphase = atan2( imag, real ); /* the result is between -M_PI and MP_PI */
      }
      /* When the atom and its conjugate are aligned, they should be real 
       * and the phase is simply the sign of the inner product (re,im) = (re,0) */
      else {
#ifndef NDEBUG
	assert( reCorrelChirp[freqIdx] == 1.0 );
	assert( imCorrelChirp[freqIdx] == 0.0 );
	assert( im == 0 );
#endif
	amp = sqrt( energy );
	if   ( re >= 0 ) atomphase = 0.0;  /* corresponds to the '+' sign */
	else             atomphase = M_PI; /* corresponds to the '-' sign exp(i\pi) */
      }

      /* 5) fill in the atom parameters */
      gatom->amp[chanIdx]   = (MP_Real_t)( amp   );
      gatom->phase[chanIdx] = (MP_Real_t)( atomphase );
#ifndef NDEBUG
      fprintf( stderr, "mptk DEBUG -- freq %g chirp %g amp %g phase %g\n reCorrelChirp %g"
	       " imCorrelChirp %g\n re %g im %g 2*(re^2+im^2) %g\n",
	       gatom->freq, gatom->chirp, gatom->amp[chanIdx], gatom->phase[chanIdx],
	       reCorrelChirp[freqIdx], imCorrelChirp[freqIdx], re, im, 2*(re*re+im*im) );
#endif
    }
  
    /* Shall we also adjust the scale ? */

    return( 1 );
}

/******************************************************/
int MP_Chirp_Block_c::set_chirp_demodulator( MP_Real_t chirprate ) {

  unsigned long int t;
  MP_Real_t argument;
  MP_Real_t C;

  /* Update the real and imaginary parts of exp(-i*pi*chirprate*t^2) */
  C = MP_PI*chirprate;
  for ( t = 0; t < filterLen; t++ ) {
    argument = C*t*t;
    chirpRe[t] = cos( argument );
    chirpIm[t] = -sin( argument );
  }

  /* Compute the FFT of  window[t]*window[t]*exp(+2*i*pi*chirprate*t^2)
     (one multiplication by window is done within exec_complex) */
  for (t = 0; t < filterLen; t++) {
    sigChirpRe[t] =  (chirpRe[t]*chirpRe[t]-chirpIm[t]*chirpIm[t]);
    sigChirpIm[t] = -2*chirpRe[t]*chirpIm[t];
  }
  fft->exec_complex_demod( fft->window, sigChirpRe, sigChirpIm, fftRe, fftIm );

  /* 3/ Fill reCorrelChirp and imCorrelChirp with the adequate FFT values: */
  if ( fill_correl( reCorrelChirp, imCorrelChirp, sqCorrelChirp, cstCorrelChirp ) ) {
    fprintf( stderr, "mplib warning -- set_chirp_demodulator() - "
	     "The tabulation of the atom's autocorrelations returned an error.\n" );
  }

  return( 0 );
}

/*************/
/* FUNCTIONS */
/*************/

/************************************************/
/* Addition of one chirp block to a dictionnary */
int add_chirp_block( MP_Dict_c *dict,
		     const unsigned long int filterLen,
		     const unsigned long int filterShift,
		     const unsigned long int fftRealSize,
		     const unsigned char windowType,
		     const double windowOption,
		     const unsigned char numFitPoints,
		     const unsigned int numIter ) {

  MP_Chirp_Block_c *newBlock;
  
  if( 2*(fftRealSize-1) < filterLen) {
    fprintf( stderr, "mplib error -- add_chirp_block() - fftRealSize %lu is too small"
	     " since window size is %lu.\n", fftRealSize, filterLen);
    return( 0 );
  }

  newBlock = new MP_Chirp_Block_c( dict->signal, filterLen, filterShift, fftRealSize,
				   windowType, windowOption ,
				   numFitPoints, numIter );
  if ( newBlock != NULL ) {
    dict->add_block( newBlock );
  }
  else {
    fprintf( stderr, "mplib error -- add_chirp_block() - Can't add a new chirp block"
	     " to a dictionnary.\n" );
    return( 0 );
  }

  return( 1 );

}


/*****************************************************/
/* Addition of several chirp blocks to a dictionnary */
int add_chirp_blocks( MP_Dict_c *dict,
		      const unsigned long int maxFilterLen,
		      const MP_Real_t timeDensity,
		      const MP_Real_t freqDensity, 
		      const unsigned char setWindowType,
		      const double setWindowOption,
		      const unsigned char setNumFitPoints,
		      const unsigned int setNumIter ) {

  unsigned long int setFilterLen;
  unsigned long int setFilterShift;
  unsigned long int setFftRealSize;
  int nAddedBlocks = 0;

#ifndef NDEBUG
  assert(timeDensity > 0.0);
  assert(freqDensity > 0.0);
#endif

  for ( setFilterLen = 4; setFilterLen <= maxFilterLen; setFilterLen <<= 1 ) {

    setFilterShift = (unsigned long int)round((MP_Real_t)setFilterLen/timeDensity);
    setFftRealSize = (unsigned long int)round((MP_Real_t)(setFilterLen/2+1)*freqDensity);
    nAddedBlocks += add_chirp_block( dict,
				     setFilterLen, setFilterShift, setFftRealSize,
				     setWindowType, setWindowOption ,
				     setNumFitPoints, setNumIter );
  }

  return(nAddedBlocks);
}
