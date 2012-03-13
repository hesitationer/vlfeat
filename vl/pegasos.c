/** @file pegasos.c
 ** @brief PEGASOS - Definition
 ** @author Andrea Vedaldi
 **/

/*
Copyright (C) 2007-12 Andrea Vedaldi and Brian Fulkerson.
All rights reserved.

This file is part of the VLFeat library and is made available under
the terms of the BSD license (see the COPYING file).
*/

/**
<!-- ------------------------------------------------------------- -->
@page pegasos PEGASOS SVM solver
@author Andrea Vedaldi
<!-- ------------------------------------------------------------- -->

@ref pegasos.h provides a basic implementation of the PEGASOS
@cite{shalev-shwartz07pegasos} linear SVM solver.

- @ref pegasos-overview "Overview"
  - @ref pegasos-algorithm "Algorithm"
  - @ref pegasos-bias "Bias"
  - @ref pegasos-restarting "Restarting"
  - @ref pegasos-kernels "Non-linear kernels"

<!-- ------------------------------------------------------------ --->
@section pegasos-overview Overview
<!-- ------------------------------------------------------------ --->

PEGASOS solves the <em>linear</em> SxVM learning problem

@f[
 \min_{w} \frac{\lambda}{2} \|w\|^2 + \frac{1}{m} \sum_{i=1}^n
 \ell(w; (x_i,y_i))
@f]

where @f$ x_i @f$ are data vectors in @f$ \mathbb{R}^d @f$, @f$ y_i
\in \{-1,1\} @f$ are binary labels, @f$ \lambda > 0 @f$ is the
regularization parameter, and

@f[
\ell(w;(x_i,y_i)) = \max\{0, 1 - y_i \langle w,x_i\rangle\}.
@f]

is the <em>hinge loss</em>. The result of the optimization is a model
@f$ w \in \mathbb{R}^d @f$ that yields the decision function

@f[ F(x) = \operatorname{sign} \langle w, x\rangle. @f]

It is well known that the hinge loss is a convex upper bound of the
i01-loss of the decision function:

@f[
 \ell(w;(x,y)) \geq \frac{1}{2}(1 - y F(x)).
@f]

PEGASOS is accessed by calling ::vl_pegasos_train_binary_svm_d or
 ::vl_pegasos_train_binary_svm_f, operating respectively on @c double
 or @c float data.

<!-- ------------------------------------------------------------ --->
@subsection pegasos-algorithm Algorithm
<!-- ------------------------------------------------------------ --->

PEGASOS @cite{shalev-shwartz07pegasos} is a stochastic subgradient
optimizer. At the <em>t</em>-th iteration the algorithm:

- Samples uniformly at random as subset @f$ A_t@f$ of <em>k</em> of
  training pairs @f$(x,y)@f$ from the <em>m</em> pairs provided for
  training (this subset is called mini batch).
- Computes a subgradient @f$ \nabla_t @f$ of the function @f$ E_t(w) =
  \frac{1}{2}\|w\|^2 + \frac{1}{k} \sum_{(x,y) \in A_t} \ell(w;(x,y))
  @f$ (this is the SVM objective function restricted to the
  minibatch).
- Compute an intermediate weight vector @f$ w_{t+1/2} @f$ by doing a
  step @f$ w_{t+1/2} = w_t - \alpha_t \nalba_t @f$ with learning rate
  @f$ \alpha_t = 1/(\eta t) @f$ along the subgradient. Note that the
  learning rate is inversely proportional to the iteration numeber.
- Back projects the weight vector @f$ w_{t+1/2} @f$ on the
  hypersphere of radius @f$ \sqrt{\lambda} @f$ to obtain the next
  model estimate @f$ w_{t+1} @f$:
  @f[
     w_t = \min\{1, \sqrt{\lambda}/\|w\|\} w_{t+1/2}.
  @f]
  The hypersfere is guaranteed to contain the optimal weight vector
  @f$ w^* @f$ @cite{shalev-shwartz07pegasos}.

VLFeat implementation fixes to one the size of the mini batches @f$ k
@f$.

<!-- ------------------------------------------------------------ --->
@subsection pegasos-bias Bias
<!-- ------------------------------------------------------------ --->

PEGASOS SVM formulation does not incorporate a bias. To learn an SVM
with bias, the each data vector @f$ x @f$ can be extended by a
constant component @f$ B @f$ (called @c biasMultiplier in the
code). In this case, the model @f$ w @f$ has dimension @f$ D + 1 @f$
and the SVM discriminat function is given by @f$ F(x) =
\operatorname{sign} (\langle w_{1:d}, x\rangle+ w_{d+1} B) @f$. If the
bias multiplier <em>B</em> is large enough, the weight @f$ w_{d+1} @f$
remains small and it has small contribution in the SVM regularization
term @f$ \| w \|^2 @f$, better approximating the case of an SVM with
bias. Unfortunately, setting the bias multiplier @f$ B @f$ to a large
value makes the optimization harder.

<!-- ------------------------------------------------------------ --->
@subsection pegasos-restarting Restarting
<!-- ------------------------------------------------------------ --->

VLFeat PEGASOS implementation can be restatred after any given number
of iterations. This is useful to compute intermediate statistics or to
load new data from disk for large datasets.  The state of the
algorithm, which is required for restarting, is limited to the current
estimate @f$ w_t @f$ of the SVM weight vector and the iteration number
@f$ t @f$.

<!-- ------------------------------------------------------------ --->
@subsection pegasos-permutation Permutation
<!-- ------------------------------------------------------------ --->

VLFeat PEGASOS can use a user-defined permutation to decide the order
in which data points are visited (instead of using random
sampling). By specifying a permutation the algorithm is guaranteed to
visit each data point exactly once in each loop. The permutation needs
not to be bijective. This can be used to visit certain data samples
more or less often than others, implicitly reweighting their relative
importance in the SVM objective function. This can be used to blanace
the data.

<!-- ------------------------------------------------------------ --->
@subsection pegasos-kernels Non-linear kernels
<!-- ------------------------------------------------------------ --->

PEGASOS can be extended to non-linear kernels, but the algorithm is
not particularly efficient in this setting [1]. When possible, it may
be preferable to work with explicit feature maps.

Let @f$ k(x,y) @f$ be a positive definite kernel. A <em>feature
map</em> is a function @f$ \Psi(x) @f$ such that @f$ k(x,y) = \langle
\Psi(x), \Psi(y) \rangle @f$. Using this representation the non-linear
SVM learning objective function writes:

@f[
 \min_{w} \frac{\lambda}{2} \|w\|^2 + \frac{1}{m} \sum_{i=1}^n
 \ell(w; (\Psi(x)_i,y_i)).
@f]

Thus the only difference with the linear case is that the feature @f$
\Psi(x) @f$ is used in place of the data @f$ x @f$.

@f$ \Psi(x) @f$ can be learned off-line, for instance by using the
incomplete Cholesky decomposition @f$ V^\top V @f$ of the Gram matrix
@f$ K = [k(x_i,x_j)] @f$ (in this case @f$ \Psi(x_i) @f$ is the
<em>i</em>-th columns of <em>V</em>). Alternatively, for additive
kernels (e.g. intersection, Chi2) the explicit feature map computed by
@ref homkermap.h can be used.

*/

/** @fn vl_pegasos_train_binary_svm_d(double*,double const*,vl_size,vl_size,vl_int8 const*,double,double,vl_uindex,vl_size,VlRand*,vl_uint32 const*,vl_size,double const*)
 ** @param model (out) the learned model.
 ** @param data training vectors.
 ** @param dimension data dimension.
 ** @param numSamples number of training data vectors.
 ** @param labels labels of the training vectors.
 ** @param regularizer value of the regularizer coefficient @f$ \lambda @f$.
 ** @param biasMultiplier value of the bias multiplier @f$ B @f$.
 ** @param startingIteration number of the first iteration.
 ** @param numIterations number of iterations to perform.
 ** @param randomGenerator random number generator.
 ** @param permutation order in which the data is accessed.
 ** @param permutationSize length of @c permutation.
 ** @param preconditioner diagonal precoditioner.
 **
 ** The function runs PEGASOS on the specified data. The vector @a
 ** model must have either dimension equal to @a dimension if @a
 ** biasMultiplier is zero, or @a dimension + 1 if @a biasMultiplier
 ** is larger than zero.
 **
 ** The function runs PEGASOS for iterations <em>t</em> in the
 ** interval [@a fistIteration, @a lastIteration]. Together with the
 ** fact that the initial model can be set arbitrarily, this enable
 ** restarting PEGASOS from any point.
 **
 ** PEGASOS select the next point for computing the gradient at
 ** random. If @a randomGenerator is @c NULL, the default random
 ** generator (as returned by ::vl_get_rand()) is used.
 **
 ** Alternatively, if @a permutation is not @c NULL, then points are
 ** sampled in the order specified by this vector of indexes (this is
 ** cycled through). In this way It is an error to set both @a
 ** randomGenerator and @a permutation to non-null values.
 **
 ** @c preconditioner specifies a diagonal preconditioner for the
 ** minimization problem (it is often useful to slow down the steps
 ** for the bias term, if the latter is used). Set @c preconditioner to NULL
 ** to avoid using a preconditioner. The precodnitioner should have the
 ** same dimension of the model, plus one if an SVM with bias is learned.
 **
 ** See the @ref pegasos-overview overview for details.
 **/

/** @fn vl_pegasos_train_binary_svm_f(float*,float const*,vl_size,vl_size,vl_int8 const*,double,double,vl_uindex,vl_size,VlRand*,vl_uint32 const*,vl_size,float const*)
 ** @see ::vl_pegasos_train_binary_svm_d
 **/

#include "pegasos.h"
#include "mathop.h"
#include <math.h>

void
vl_svm_compute_diagnostics(VlSvmPegasos *svm,
                           void * data,
                           vl_size numSamples,
                           vl_int8 const * labels,
                           VlSvmInnerProductFunction innerProduct)
{
  vl_size i, k ;
  vl_size numPos = 0 ;
  vl_size numNeg = 0 ;
  double pd ;
  svm->objective->regularizer = 0.0 ;

  for (i = 0; i < svm->dimension; i++)
    {
      svm->objective->regularizer += svm->model[i] * svm->model[i] ;
    }

  svm->objective->regularizer *= svm->lambda * 0.5 ;

  svm->objective->lossPos = 0 ;
  svm->objective->lossNeg = 0 ;
  svm->objective->hardLossPos = 0 ;
  svm->objective->hardLossNeg = 0 ;

  for (k = 0; k < numSamples; k++)
    {
      pd = innerProduct(data,k,svm->model) ;
      if (svm->biasMultiplier)
	{
	  pd += svm->model[svm->dimension]*svm->biasMultiplier ;
	}

      pd = VL_MAX(1 - labels[k]*pd, 0.0) ;

      if (labels[k] < 0)
	{
	  svm->objective->lossNeg += pd ;
	  svm->objective->hardLossNeg += (pd > 0) ;
	  numNeg++ ;
	}
      else
	{
	  svm->objective->lossPos += pd ;
	  svm->objective->hardLossPos += (pd > 0) ;
	  numPos++ ;
	}
    }

  svm->objective->lossNeg /= numNeg ;
  svm->objective->hardLossNeg /= numNeg ;

  svm->objective->lossPos /= numPos ;
  svm->objective->hardLossPos /= numPos ;

  svm->objective->energy = svm->objective->regularizer + svm->objective->lossPos + svm->objective->lossNeg ;
}


VL_EXPORT
VlSvmPegasosSolver* vl_svmpegasos_new (vl_size dimension,
                                       double lambda)
{
  VlSvmPegasos  * svm ;
  svm = (VlSvmPegasos*) vl_malloc(sizeof(VlSvmPegasos)) ;

  svm->model = (double*) vl_calloc(dimension, sizeof(double)) ;
  svm->dimension = dimension ;

  svm->objective = (VlSvmObjective*) vl_malloc(sizeof(VlSvmObjective)) ;

  
  svm->maxIterations = (vl_size) 10 / (lambda + 1) ;

  assert(lambda > 0.0) ;

  svm->lambda = lambda ;


  svm->epsilon = -1 ;
  svm->biasMultiplier = 0 ;
  svm->bias = 0 ;

  svm->biasLearningRate = 1 ;

  svm->energyCheckFrequency = 100 ;

  svm->randomGenerator = NULL ; 
}

VL_EXPORT
void vl_pegasos_delete (VlSvmPegasos * svm)
{
  if (svm->model)
    vl_free(svm->model) ;

  if (svm->objective)
    vl_free(svm->objective) ;

  if (svm->preConditioner)
    vl_free(svm->preConditioner) ;

  if (svm->permutation)
    vl_free(svm->permutation) ;

  vl_free(svm) ;
}


VL_EXPORT void
vl_svmpegasos_train(VlSvmPegasos * svm,
                    void * data,
                    vl_size numSamples,
                    vl_int8 const * labels,
                    VlSvmInnerProductFunction innerProduct,
                    VlSvmAccumulatorFunction accumulator,
                    void * validation,
                    vl_size validationNumSamples,
                    vl_int8 const * validationLabels,
                    VlSvmDiagnostics diagnostics,
                    void * diagnosticCallerRef)
{
  vl_tic() ;
  vl_uindex iteration0 ;

  double energy ;

  vl_size i ;

  double acc, learningRate, y ;
  double lambda = svm->lambda ;
  vl_size const regularizationPeriod = 10 ;


  if (svm->randomGenerator == NULL && svm->permutation == NULL) {
    svm->randomGenerator = vl_get_rand() ;
  }


  assert(svm->randomGenerator == NULL || svm->permutation == NULL) ;
  //assert(svm->iterationsSoFar >= 0) ;

  /*
   Choose iteration0 to start with small enoguh steps. Recall that
   the learning rate is

   learningRate = 1 / (lambda * (iteration + iteration0))

   */

  iteration0 = (vl_uindex) 1.0 / lambda ;

  /*
   The model is stored as scale*model[]. When a sample does not violate
   the margin, only scale needs to be updated.
   */

  for ( ++(svm->iterationsSoFar) ;
       svm->iterationsSoFar <  svm->maxIterations ;
       ++ svm->iterationsSoFar)
    {
      /* pick a sample  */
      vl_uindex k ;
      if (svm->permutation == NULL) {
	k = vl_rand_uindex(randomGenerator, numSamples) ;
      } else {
	k = svm->permutation[svm->iterationsSoFar % svm->permutationSize] ;
	assert(k < numSamples) ;
      }


      y = labels[k] ;

      /* compute learning rate */
      learningRate = 1.0 / ((svm->iterationsSoFar + iteration0) * lambda) ;

      /* regularizer step ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
      if (svm->iterationsSoFar % regularizationPeriod == 0) {
	double eta =  learningRate * regularizationPeriod * lambda ;


	
        for (i = 0 ; i < svm->dimension  ; ++i)
          {
            svm->model[i] -= eta * svm->model[i] ;
          }
        if (svm->biasMultiplier)
          svm->bias -= eta * svm->biasLearningRate * svm->bias ;
	  

      }

      /* loss step ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
      acc = innerProduct(data,k,svm->model) ;

      if (svm->biasMultiplier)
	acc += svm->biasMultiplier * svm->bias ;

      if (y * acc < (double) 1.0) {
	double eta = y * learningRate ;

	acc = 0 ;

	accumulator(data,k,svm->model,eta) ;


	if (svm->biasMultiplier)
	  {
            svm->bias += eta * svm->biasLearningRate * svm->biasMultiplier ;
	  }

      }

      if (svm->iterationsSoFar % svm->energyCheckFrequency == 0)
	{
	  svm->elapsedTime += vl_toc() ;
          if(validation)
            vl_svm_compute_diagnostics(svm,
                                       validation,
                                       validationNumSamples,
                                       validationLabels,
                                       innerProduct) ;
          else
            vl_svm_compute_diagnostics(svm,
                                       data,
                                       numSamples,
                                       labels,
                                       innerProduct) ;


          if (diagnostics)
            diagnostics(diagnosticCallerRef,svm) ;

          if(svm->epsilon > 0 && abs(energy - svm->objective->energy) < svm->epsilon)
            break ;

	  vl_tic() ;
	}

    }


  svm->elapsedTime += vl_toc() ;
}
