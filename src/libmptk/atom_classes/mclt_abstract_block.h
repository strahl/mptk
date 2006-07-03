/******************************************************************************/
/*                                                                            */
/*                          mclt_abstract_block.h                             */
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

/****************************************************************/
/*                                                		*/
/* DEFINITION OF THE mclt_abstract BLOCK CLASS            	*/
/* RELEVANT TO THE mclt_abstract TIME-FREQUENCY TRANSFORM 	*/
/*                                                		*/
/****************************************************************/


#ifndef __mclt_abstract_block_h_
#define __mclt_abstract_block_h_


/********************************/
/* mclt_abstract BLOCK CLASS    */
/********************************/

class MP_Mclt_Abstract_Block_c:public MP_Block_c {

  /********/
  /* DATA */
  /********/

public:
  /** \brief FFT interface, which includes the window with which the signal is multiplied */
  MP_FFT_Interface_c *fft;

  /* The FFT size */
  unsigned long int fftSize;

  /* The number of frequencies for the mclt/mdct/mdst: fftSize/2 */
  unsigned long int numFreqs;

  /** \brief A (fft->numFreqs x s->numChans) array which holds the frame-wise FFT results
      across channels */
  MP_Real_t *mag;

  /* A couple of buffers of size numFreqs to perform complex fft computations
     in create_atom. */
  MP_Real_t *fftRe;
  MP_Real_t *fftIm;

  /* A couple of buffers of size numFreqs for the output of the mclt transform */
  MP_Real_t *mcltOutRe;
  MP_Real_t *mcltOutIm;

  /* A couple of buffers to perform pre-modulation on the input frame */
  MP_Real_t *preModRe;
  MP_Real_t *preModIm;

  /* A couple of buffers to perform post-modulation on the fft output*/
  MP_Real_t *postModRe;
  MP_Real_t *postModIm;

  /***********/
  /* METHODS */
  /***********/

  /***************************/
  /* CONSTRUCTORS/DESTRUCTOR */
  /***************************/

public:

  /** \brief an initializer for the parameters which ARE related to the signal */
  virtual int plug_signal( MP_Signal_c *setSignal );

protected:
  /** \brief an initializer for the parameters which ARE NOT related to the signal */
  virtual int init_parameters( const unsigned long int setFilterLen,
			       const unsigned long int setFilterShift,
			       const unsigned long int setFftSize,
			       const unsigned char setWindowType,
			       const double setWindowOption );

  /** \brief nullification of the signal-related parameters */
  virtual void nullify_signal( void );

  /** \brief a constructor which initializes everything to zero or NULL */
  MP_Mclt_Abstract_Block_c( void );

public:

  /* Destructor */
  virtual ~MP_Mclt_Abstract_Block_c();


  /***************************/
  /* OTHER METHODS           */
  /***************************/

public:

  int check_fftsize( const unsigned long int setFilterLen, const unsigned long int setFftSize );

  virtual void init_transform( void );

  virtual void compute_transform(MP_Real_t *in);
};
 

#endif /* __mclt_abstract_block_h_ */
