// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2008 Gael Guennebaud <g.gael@free.fr>
// Copyright (C) 2010 Jitse Niesen <jitse@maths.leeds.ac.uk>
//
// Eigen is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or (at your option) any later version.
//
// Alternatively, you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// Eigen is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License or the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License and a copy of the GNU General Public License along with
// Eigen. If not, see <http://www.gnu.org/licenses/>.

#ifndef EIGEN_SELFADJOINTEIGENSOLVER_H
#define EIGEN_SELFADJOINTEIGENSOLVER_H

/** \eigenvalues_module \ingroup Eigenvalues_Module
  * \nonstableyet
  *
  * \class SelfAdjointEigenSolver
  *
  * \brief Computes eigenvalues and eigenvectors of selfadjoint matrices
  *
  * \tparam _MatrixType the type of the matrix of which we are computing the
  * eigendecomposition; this is expected to be an instantiation of the Matrix
  * class template. Currently, only real matrices are supported.
  *
  * A matrix \f$ A \f$ is selfadjoint if it equals its adjoint. For real
  * matrices, this means that the matrix is symmetric: it equals its
  * transpose. This class computes the eigenvalues and eigenvectors of a
  * selfadjoint matrix. These are the scalars \f$ \lambda \f$ and vectors 
  * \f$ v \f$ such that \f$ Av = \lambda v \f$.  The eigenvalues of a
  * selfadjoint matrix are always real. If \f$ D \f$ is a diagonal matrix with
  * the eigenvalues on the diagonal, and \f$ V \f$ is a matrix with the
  * eigenvectors as its columns, then \f$ A = V D V^{-1} \f$ (for selfadjoint
  * matrices, the matrix \f$ V \f$ is always invertible). This is called the
  * eigendecomposition.
  *
  * The algorithm exploits the fact that the matrix is selfadjoint, making it
  * faster and more accurate than the general purpose eigenvalue algorithms
  * implemented in EigenSolver and ComplexEigenSolver.
  *
  * This class can also be used to solve the generalized eigenvalue problem
  * \f$ Av = \lambda Bv \f$. In this case, the matrix \f$ A \f$ should be
  * selfadjoint and the matrix \f$ B \f$ should be positive definite.
  *
  * Call the function compute() to compute the eigenvalues and eigenvectors of
  * a given matrix. Alternatively, you can use the
  * SelfAdjointEigenSolver(const MatrixType&, bool) constructor which computes
  * the eigenvalues and eigenvectors at construction time. Once the eigenvalue
  * and eigenvectors are computed, they can be retrieved with the eigenvalues()
  * and eigenvectors() functions.
  *
  * The documentation for SelfAdjointEigenSolver(const MatrixType&, bool)
  * contains an example of the typical use of this class.
  *  
  * \sa MatrixBase::eigenvalues(), class EigenSolver, class ComplexEigenSolver
  */
template<typename _MatrixType> class SelfAdjointEigenSolver
{
  public:

    typedef _MatrixType MatrixType;
    enum {
      Size = MatrixType::RowsAtCompileTime,
      ColsAtCompileTime = MatrixType::ColsAtCompileTime,
      Options = MatrixType::Options,
      MaxColsAtCompileTime = MatrixType::MaxColsAtCompileTime
    };

    /** \brief Scalar type for matrices of type \p _MatrixType. */
    typedef typename MatrixType::Scalar Scalar;
    typedef typename MatrixType::Index Index;

    /** \brief Real scalar type for \p _MatrixType. 
      *
      * This is just \c Scalar if #Scalar is real (e.g., \c float or 
      * \c double), and the type of the real part of \c Scalar if #Scalar is
      * complex.
      */
    typedef typename NumTraits<Scalar>::Real RealScalar;

    /** \brief Type for vector of eigenvalues as returned by eigenvalues(). 
      *
      * This is a column vector with entries of type #RealScalar.
      * The length of the vector is the size of \p _MatrixType.
      */
    typedef typename ei_plain_col_type<MatrixType, RealScalar>::type RealVectorType;
    typedef Tridiagonalization<MatrixType> TridiagonalizationType;

    /** \brief Default constructor for fixed-size matrices.
      *
      * The default constructor is useful in cases in which the user intends to
      * perform decompositions via compute(const MatrixType&, bool) or
      * compute(const MatrixType&, const MatrixType&, bool). This constructor
      * can only be used if \p _MatrixType is a fixed-size matrix; use
      * SelfAdjointEigenSolver(Index) for dynamic-size matrices.
      *
      * Example: \include SelfAdjointEigenSolver_SelfAdjointEigenSolver.cpp
      * Output: \verbinclude SelfAdjointEigenSolver_SelfAdjointEigenSolver.out
      */
    SelfAdjointEigenSolver()
        : m_eivec(),
          m_eivalues(),
          m_tridiag(),
          m_subdiag()
    {
      ei_assert(Size!=Dynamic);
    }

    /** \brief Constructor, pre-allocates memory for dynamic-size matrices.
      *
      * \param [in]  size  Positive integer, size of the matrix whose
      * eigenvalues and eigenvectors will be computed.
      *
      * This constructor is useful for dynamic-size matrices, when the user
      * intends to perform decompositions via compute(const MatrixType&, bool)
      * or compute(const MatrixType&, const MatrixType&, bool). The \p size
      * parameter is only used as a hint. It is not an error to give a wrong 
      * \p size, but it may impair performance.
      *
      * \sa compute(const MatrixType&, bool) for an example
      */
    SelfAdjointEigenSolver(Index size)
        : m_eivec(size, size),
          m_eivalues(size),
          m_tridiag(size),
          m_subdiag(size > 1 ? size - 1 : 1)
    {}

    /** \brief Constructor; computes eigendecomposition of given matrix. 
      * 
      * \param[in]  matrix  Selfadjoint matrix whose eigendecomposition is to
      *    be computed. 
      * \param[in]  computeEigenvectors  If true, both the eigenvectors and the
      *    eigenvalues are computed; if false, only the eigenvalues are
      *    computed. 
      *
      * This constructor calls compute(const MatrixType&, bool) to compute the
      * eigenvalues of the matrix \p matrix. The eigenvectors are computed if
      * \p computeEigenvectors is true.
      *
      * Example: \include SelfAdjointEigenSolver_SelfAdjointEigenSolver_MatrixType.cpp
      * Output: \verbinclude SelfAdjointEigenSolver_SelfAdjointEigenSolver_MatrixType.out
      *
      * \sa compute(const MatrixType&, bool), 
      *     SelfAdjointEigenSolver(const MatrixType&, const MatrixType&, bool)
      */
    SelfAdjointEigenSolver(const MatrixType& matrix, bool computeEigenvectors = true)
      : m_eivec(matrix.rows(), matrix.cols()),
        m_eivalues(matrix.cols()),
        m_tridiag(matrix.rows()),
        m_subdiag(matrix.rows() > 1 ? matrix.rows() - 1 : 1)
    {
      compute(matrix, computeEigenvectors);
    }

    /** \brief Constructor; computes eigendecomposition of given matrix pencil.
      * 
      * \param[in]  matA  Selfadjoint matrix in matrix pencil.
      * \param[in]  matB  Positive-definite matrix in matrix pencil.
      * \param[in]  computeEigenvectors  If true, both the eigenvectors and the
      *    eigenvalues are computed; if false, only the eigenvalues are
      *    computed. 
      *
      * This constructor calls compute(const MatrixType&, const MatrixType&, bool) 
      * to compute the eigenvalues and (if requested) the eigenvectors of the
      * generalized eigenproblem \f$ Ax = \lambda B x \f$ with \a matA the
      * selfadjoint matrix \f$ A \f$ and \a matB the positive definite matrix
      * \f$ B \f$ . The eigenvectors are computed if \a computeEigenvectors is
      * true.
      *
      * Example: \include SelfAdjointEigenSolver_SelfAdjointEigenSolver_MatrixType2.cpp
      * Output: \verbinclude SelfAdjointEigenSolver_SelfAdjointEigenSolver_MatrixType2.out
      *
      * \sa compute(const MatrixType&, const MatrixType&, bool), 
      *     SelfAdjointEigenSolver(const MatrixType&, bool)
      */
    SelfAdjointEigenSolver(const MatrixType& matA, const MatrixType& matB, bool computeEigenvectors = true)
      : m_eivec(matA.rows(), matA.cols()),
        m_eivalues(matA.cols()),
        m_tridiag(matA.rows()),
        m_subdiag(matA.rows() > 1 ? matA.rows() - 1 : 1)
    {
      compute(matA, matB, computeEigenvectors);
    }

    /** \brief Computes eigendecomposition of given matrix. 
      * 
      * \param[in]  matrix  Selfadjoint matrix whose eigendecomposition is to
      *    be computed. 
      * \param[in]  computeEigenvectors  If true, both the eigenvectors and the
      *    eigenvalues are computed; if false, only the eigenvalues are
      *    computed. 
      * \returns    Reference to \c *this
      *
      * This function computes the eigenvalues of \p matrix.  The eigenvalues()
      * function can be used to retrieve them.  If \p computeEigenvectors is
      * true, then the eigenvectors are also computed and can be retrieved by
      * calling eigenvectors().
      *
      * This implementation uses a symmetric QR algorithm. The matrix is first
      * reduced to tridiagonal form using the Tridiagonalization class. The
      * tridiagonal matrix is then brought to diagonal form with implicit
      * symmetric QR steps with Wilkinson shift. Details can be found in
      * Section 8.3 of Golub \& Van Loan, <i>%Matrix Computations</i>.
      *
      * The cost of the computation is about \f$ 9n^3 \f$ if the eigenvectors
      * are required and \f$ 4n^3/3 \f$ if they are not required.
      *
      * This method reuses the memory in the SelfAdjointEigenSolver object that
      * was allocated when the object was constructed, if the size of the
      * matrix does not change.
      *
      * Example: \include SelfAdjointEigenSolver_compute_MatrixType.cpp
      * Output: \verbinclude SelfAdjointEigenSolver_compute_MatrixType.out
      *
      * \sa SelfAdjointEigenSolver(const MatrixType&, bool)
      */
    SelfAdjointEigenSolver& compute(const MatrixType& matrix, bool computeEigenvectors = true);

    /** \brief Computes eigendecomposition of given matrix pencil. 
      * 
      * \param[in]  matA  Selfadjoint matrix in matrix pencil.
      * \param[in]  matB  Positive-definite matrix in matrix pencil.
      * \param[in]  computeEigenvectors  If true, both the eigenvectors and the
      *    eigenvalues are computed; if false, only the eigenvalues are
      *    computed. 
      * \returns    Reference to \c *this
      *
      * This function computes eigenvalues and (if requested) the eigenvectors
      * of the generalized eigenproblem \f$ Ax = \lambda B x \f$ with \a matA
      * the selfadjoint matrix \f$ A \f$ and \a matB the positive definite
      * matrix \f$ B \f$. The eigenvalues() function can be used to retrieve
      * the eigenvalues.  If \p computeEigenvectors is true, then the
      * eigenvectors are also computed and can be retrieved by calling
      * eigenvectors().
      *
      * The implementation uses LLT to compute the Cholesky decomposition 
      * \f$ B = LL^* \f$ and calls compute(const MatrixType&, bool) to compute
      * the eigendecomposition \f$ L^{-1} A (L^*)^{-1} \f$. This solves the
      * generalized eigenproblem, because any solution of the generalized
      * eigenproblem \f$ Ax = \lambda B x \f$ corresponds to a solution 
      * \f$ L^{-1} A (L^*)^{-1} (L^* x) = \lambda (L^* x) \f$ of the
      * eigenproblem for \f$ L^{-1} A (L^*)^{-1} \f$.
      *
      * Example: \include SelfAdjointEigenSolver_compute_MatrixType2.cpp
      * Output: \verbinclude SelfAdjointEigenSolver_compute_MatrixType2.out
      *
      * \sa SelfAdjointEigenSolver(const MatrixType&, const MatrixType&, bool)
      */
    SelfAdjointEigenSolver& compute(const MatrixType& matA, const MatrixType& matB, bool computeEigenvectors = true);

    /** \brief Returns the eigenvectors of given matrix (pencil). 
      *
      * \returns  A const reference to the matrix whose columns are the eigenvectors.
      *
      * \pre The eigenvectors have been computed before.
      *
      * Column \f$ k \f$ of the returned matrix is an eigenvector corresponding
      * to eigenvalue number \f$ k \f$ as returned by eigenvalues().  The
      * eigenvectors are normalized to have (Euclidean) norm equal to one. If
      * this object was used to solve the eigenproblem for the selfadjoint
      * matrix \f$ A \f$, then the matrix returned by this function is the
      * matrix \f$ V \f$ in the eigendecomposition \f$ A = V D V^{-1} \f$.
      *
      * Example: \include SelfAdjointEigenSolver_eigenvectors.cpp
      * Output: \verbinclude SelfAdjointEigenSolver_eigenvectors.out
      *
      * \sa eigenvalues()
      */
    const MatrixType& eigenvectors() const
    {
      #ifndef NDEBUG
      ei_assert(m_eigenvectorsOk);
      #endif
      return m_eivec;
    }

    /** \brief Returns the eigenvalues of given matrix (pencil). 
      *
      * \returns A const reference to the column vector containing the eigenvalues.
      *
      * \pre The eigenvalues have been computed before.
      *
      * The eigenvalues are repeated according to their algebraic multiplicity,
      * so there are as many eigenvalues as rows in the matrix.
      *
      * Example: \include SelfAdjointEigenSolver_eigenvalues.cpp
      * Output: \verbinclude SelfAdjointEigenSolver_eigenvalues.out
      *
      * \sa eigenvectors(), MatrixBase::eigenvalues()
      */
    const RealVectorType& eigenvalues() const { return m_eivalues; }

    /** \brief Computes the positive-definite square root of the matrix. 
      *
      * \returns the positive-definite square root of the matrix
      *
      * \pre The eigenvalues and eigenvectors of a positive-definite matrix
      * have been computed before.
      *
      * The square root of a positive-definite matrix \f$ A \f$ is the
      * positive-definite matrix whose square equals \f$ A \f$. This function
      * uses the eigendecomposition \f$ A = V D V^{-1} \f$ to compute the
      * square root as \f$ A^{1/2} = V D^{1/2} V^{-1} \f$.
      *
      * Example: \include SelfAdjointEigenSolver_operatorSqrt.cpp
      * Output: \verbinclude SelfAdjointEigenSolver_operatorSqrt.out
      *
      * \sa operatorInverseSqrt(), 
      *     \ref MatrixFunctions_Module "MatrixFunctions Module"
      */
    MatrixType operatorSqrt() const
    {
      return m_eivec * m_eivalues.cwiseSqrt().asDiagonal() * m_eivec.adjoint();
    }

    /** \brief Computes the inverse square root of the matrix. 
      *
      * \returns the inverse positive-definite square root of the matrix
      *
      * \pre The eigenvalues and eigenvectors of a positive-definite matrix
      * have been computed before.
      *
      * This function uses the eigendecomposition \f$ A = V D V^{-1} \f$ to
      * compute the inverse square root as \f$ V D^{-1/2} V^{-1} \f$. This is
      * cheaper than first computing the square root with operatorSqrt() and
      * then its inverse with MatrixBase::inverse().
      *
      * Example: \include SelfAdjointEigenSolver_operatorInverseSqrt.cpp
      * Output: \verbinclude SelfAdjointEigenSolver_operatorInverseSqrt.out
      *
      * \sa operatorSqrt(), MatrixBase::inverse(),
      *     \ref MatrixFunctions_Module "MatrixFunctions Module"
      */
    MatrixType operatorInverseSqrt() const
    {
      return m_eivec * m_eivalues.cwiseInverse().cwiseSqrt().asDiagonal() * m_eivec.adjoint();
    }


  protected:
    MatrixType m_eivec;
    RealVectorType m_eivalues;
    TridiagonalizationType m_tridiag;
    typename TridiagonalizationType::SubDiagonalType m_subdiag;
    #ifndef NDEBUG
    bool m_eigenvectorsOk;
    #endif
};

#ifndef EIGEN_HIDE_HEAVY_CODE

/** \internal
  *
  * \eigenvalues_module \ingroup Eigenvalues_Module
  *
  * Performs a QR step on a tridiagonal symmetric matrix represented as a
  * pair of two vectors \a diag and \a subdiag.
  *
  * \param matA the input selfadjoint matrix
  * \param hCoeffs returned Householder coefficients
  *
  * For compilation efficiency reasons, this procedure does not use eigen expression
  * for its arguments.
  *
  * Implemented from Golub's "Matrix Computations", algorithm 8.3.2:
  * "implicit symmetric QR step with Wilkinson shift"
  */
template<typename RealScalar, typename Scalar, typename Index>
static void ei_tridiagonal_qr_step(RealScalar* diag, RealScalar* subdiag, Index start, Index end, Scalar* matrixQ, Index n);

template<typename MatrixType>
SelfAdjointEigenSolver<MatrixType>& SelfAdjointEigenSolver<MatrixType>::compute(const MatrixType& matrix, bool computeEigenvectors)
{
  #ifndef NDEBUG
  m_eigenvectorsOk = computeEigenvectors;
  #endif
  assert(matrix.cols() == matrix.rows());
  Index n = matrix.cols();
  m_eivalues.resize(n,1);
  m_eivec.resize(n,n);

  if(n==1)
  {
    m_eivalues.coeffRef(0,0) = ei_real(matrix.coeff(0,0));
    m_eivec.setOnes();
    return *this;
  }

  m_tridiag.compute(matrix);
  RealVectorType& diag = m_eivalues;
  diag = m_tridiag.diagonal();
  m_subdiag = m_tridiag.subDiagonal();
  if (computeEigenvectors)
    m_eivec = m_tridiag.matrixQ();

  Index end = n-1;
  Index start = 0;
  while (end>0)
  {
    for (Index i = start; i<end; ++i)
      if (ei_isMuchSmallerThan(ei_abs(m_subdiag[i]),(ei_abs(diag[i])+ei_abs(diag[i+1]))))
        m_subdiag[i] = 0;

    // find the largest unreduced block
    while (end>0 && m_subdiag[end-1]==0)
      end--;
    if (end<=0)
      break;
    start = end - 1;
    while (start>0 && m_subdiag[start-1]!=0)
      start--;

    ei_tridiagonal_qr_step(diag.data(), m_subdiag.data(), start, end, computeEigenvectors ? m_eivec.data() : (Scalar*)0, n);
  }

  // Sort eigenvalues and corresponding vectors.
  // TODO make the sort optional ?
  // TODO use a better sort algorithm !!
  for (Index i = 0; i < n-1; ++i)
  {
    Index k;
    m_eivalues.segment(i,n-i).minCoeff(&k);
    if (k > 0)
    {
      std::swap(m_eivalues[i], m_eivalues[k+i]);
      m_eivec.col(i).swap(m_eivec.col(k+i));
    }
  }
  return *this;
}

template<typename MatrixType>
SelfAdjointEigenSolver<MatrixType>& SelfAdjointEigenSolver<MatrixType>::
compute(const MatrixType& matA, const MatrixType& matB, bool computeEigenvectors)
{
  ei_assert(matA.cols()==matA.rows() && matB.rows()==matA.rows() && matB.cols()==matB.rows());

  // Compute the cholesky decomposition of matB = L L'
  LLT<MatrixType> cholB(matB);

  // compute C = inv(L) A inv(L')
  MatrixType matC = matA;
  cholB.matrixL().solveInPlace(matC);
  // FIXME since we currently do not support A * inv(L'), let's do (inv(L) A')' :
  matC.adjointInPlace();
  cholB.matrixL().solveInPlace(matC);
  matC.adjointInPlace();
  // this version works too:
//   matC = matC.transpose();
//   cholB.matrixL().conjugate().template marked<Lower>().solveTriangularInPlace(matC);
//   matC = matC.transpose();
  // FIXME: this should work: (currently it only does for small matrices)
//   Transpose<MatrixType> trMatC(matC);
//   cholB.matrixL().conjugate().eval().template marked<Lower>().solveTriangularInPlace(trMatC);

  compute(matC, computeEigenvectors);

  if (computeEigenvectors)
  {
    // transform back the eigen vectors: evecs = inv(U) * evecs
    cholB.matrixU().solveInPlace(m_eivec);
    for (Index i=0; i<m_eivec.cols(); ++i)
      m_eivec.col(i) = m_eivec.col(i).normalized();
  }
  return *this;
}

#endif // EIGEN_HIDE_HEAVY_CODE

#ifndef EIGEN_EXTERN_INSTANTIATIONS
template<typename RealScalar, typename Scalar, typename Index>
static void ei_tridiagonal_qr_step(RealScalar* diag, RealScalar* subdiag, Index start, Index end, Scalar* matrixQ, Index n)
{
  RealScalar td = (diag[end-1] - diag[end])*RealScalar(0.5);
  RealScalar e2 = ei_abs2(subdiag[end-1]);
  RealScalar mu = diag[end] - e2 / (td + (td>0 ? 1 : -1) * ei_sqrt(td*td + e2));
  RealScalar x = diag[start] - mu;
  RealScalar z = subdiag[start];

  for (Index k = start; k < end; ++k)
  {
    PlanarRotation<RealScalar> rot;
    rot.makeGivens(x, z);

    // do T = G' T G
    RealScalar sdk = rot.s() * diag[k] + rot.c() * subdiag[k];
    RealScalar dkp1 = rot.s() * subdiag[k] + rot.c() * diag[k+1];

    diag[k] = rot.c() * (rot.c() * diag[k] - rot.s() * subdiag[k]) - rot.s() * (rot.c() * subdiag[k] - rot.s() * diag[k+1]);
    diag[k+1] = rot.s() * sdk + rot.c() * dkp1;
    subdiag[k] = rot.c() * sdk - rot.s() * dkp1;

    if (k > start)
      subdiag[k - 1] = rot.c() * subdiag[k-1] - rot.s() * z;

    x = subdiag[k];

    if (k < end - 1)
    {
      z = -rot.s() * subdiag[k+1];
      subdiag[k + 1] = rot.c() * subdiag[k+1];
    }

    // apply the givens rotation to the unit matrix Q = Q * G
    if (matrixQ)
    {
      Map<Matrix<Scalar,Dynamic,Dynamic> > q(matrixQ,n,n);
      q.applyOnTheRight(k,k+1,rot);
    }
  }
}
#endif

#endif // EIGEN_SELFADJOINTEIGENSOLVER_H