/*
 * sarray.h
 *
 *  Created on: Apr 10, 2012
 *      Author: jbalzer
 */


#ifndef SARRAY_H_
#define SARRAY_H_

#include <map>
#include <vector>
#include <stdlib.h>
#include "darray.h"

namespace R4R {

template <class T> class CSparseDiagonalArray;
template <class T> class CSparseLowerTriangularArray;
template <class T> class CSparseUpperTriangularArray;

/*! \brief sparse 2d matrix/array
 *
 *
 *
 */
template<class T>
class CSparseArray {

	friend class CSparseDiagonalArray<T>;
	friend class CSparseLowerTriangularArray<T>;
	friend class CSparseUpperTriangularArray<T>;

public:

	//! Constructor
	CSparseArray();

	//! Constructor
	CSparseArray(size_t nrows, size_t ncols);

	//! Prints the array to standard ouput.
	//void Print();

	//! Writes matrix to a stream.
	template<class U> friend std::ostream& operator << (std::ostream& os, CSparseArray<U>& x);

	//! Computes the standard L2 norm.
	T Norm2();

	//! Transposes the array in place.
	virtual void Transpose();

	//! Returns a transposed copy of the array.
	CSparseArray<T> static Transpose(const CSparseArray<T>& array);

	//! Efficiently computes the square \f$A^{\top}A\f$ of a sparse matrix \f$A\f$.
	CSparseArray<T> static Square(CSparseArray<T>& array);

	//! Non-destructive element access.
	T Get(size_t i, size_t j);

	//! Returns a row of the matrix.
	std::map<size_t,T> GetRow(size_t i);

	/*! \brief Element access.
	 *
	 *	\details Opposed to CSparseArray::Get(size_t i, size_t j) and CSparseArray::Set(size_t i, size_t j, T v),
	 *	this routine causes fill-in, as accessing a non-existing element of the map container creates
	 *	this a new element and returns a reference to it.
	 *
	 * \param[in] i row index
	 * \param[in] j column index
	 * \return reference to the element
	 *
	 */
	T& operator()(size_t i, size_t j);

	//! Sets a matrix element to a given value.
	void Set(size_t i, size_t j, T v);

	//! Multiplies the array with a scalar.
	CSparseArray<T> operator*(const T& scalar);

	//! Sums two arrays.
	CSparseArray<T> operator+(const CSparseArray<T>& array);

	//! Subtracts two arrays.
	CSparseArray<T> operator-(const CSparseArray<T>& array);

	//! Multiplies the object with a dense array from the right.
	template<class Matrix> Matrix operator*(Matrix& array);

	//! Multiplies the object with a dense vector from the right.
	CDenseVector<T> operator*(const CDenseVector<T>& vector);

	//! Computes the standard inner product.
	T static InnerProduct(CSparseArray<T>& x, CSparseArray<T>& y);

	//! Access number of cols.
	size_t NRows() const { return m_nrows; };

	//! Access number of cols.
	size_t NCols() const { return m_ncols; };

	//! In-place scalar multiplication.
	void Scale(T scalar);

	//! Counts the number of non-zero entries.
	size_t Nonzeros();

	//! Checks whether the matrix is symmetric.
	bool Symmetric();

	//! Deletes a row.
	void DeleteRow(size_t i);

	//! Deletes a column.
	void DeleteColumn(size_t j);

	//! Converts the matrix into compressed sparse row format suitable for many standard sparse solvers.
	void GetCSR(std::vector<size_t>& nz, std::vector<size_t>& j, std::vector<T>& v, bool ibase);

	//! Converts the matrix into (row,column,value) format.
	void GetCOO(std::vector<size_t>& i, std::vector<size_t>& j, std::vector<T>& v, bool ibase);

	//! Saves the matrix in matrix market format.
	bool SaveToFile(const char* filename);

	//! Sets random entries in the matrix to random values.
	void Rand(size_t nnz);

protected:


	size_t m_nrows;												//!< number of rows
	size_t m_ncols;												//!< number of cols
	bool m_transpose;											//!< transpose flag
	std::map<size_t,std::map<size_t,T> > m_data;				//!< container that holds the array data

	//! Swaps the two input indices.
	void Transpose(size_t& i, size_t& j);

};




/*! \brief sparse 2d matrix/array
 *
 *
 *
 */
template<class T>
class CSparseBandedArray {

public:

	//! Constructor
	CSparseBandedArray();

	//! Constructor
	CSparseBandedArray(size_t nrows, size_t ncols);

	//! Prints the array to standard ouput.
	void Print();

	//! Non-destructive element access.
	virtual T Get(size_t i, size_t j);

	//! Returns a range of bands.
	std::map<int,std::map<size_t,T> > GetBands(int lower, int upper);

	//! Sets a matrix element to a given value.
	virtual void Set(size_t i, size_t j, T v);

	/*! \brief Element access.
	 *
	 *
	 * \details Opposed to CSparseBandedArray::Get(size_t i, size_t j) and CSparseBandedArray::Set(size_t i, size_t j, T v),
	 * this routine causes fill-in, as accessing a non-existing element of the map container creates this a new element
	 * this and returns a reference to it.
	 *
	 * \param[in] i row index
	 * \param[in] j column index
	 *
	 * \return reference to the element
	 *
	 *
	 */
	virtual T& operator()(size_t i, size_t j);

	//! Access number of cols.
	size_t NRows() const { return m_nrows; };

	//! Access number of cols.
	size_t NCols() const { return m_ncols; };

	//! Computes the standard L2 norm.
	T Norm2();

	//! Transposes the array in place.
	virtual void Transpose();

	//! Returns a transposed copy of the array.
	CSparseBandedArray<T> static Transpose(const CSparseBandedArray<T>& array);

	/*! \brief Returns true if the array is transposed.
	 *
	 *
	 * \details No, transpose is an internal state flag only. Remove this routine for security reasons.
	 */
	bool IsTransposed() { return m_transpose; };

	//! In-place scalar multiplication.
	void Scale(T scalar);

	/*! \brief Add a scalar in-place.
	 *
	 * \details Do this only for the diagonal, otherwise the band structure will be lost.
	 *
	 */
	void AddDiagonal(const T scalar);

	//! Scale the diagonal in-place.
	void ScaleDiagonal(T scalar);

	//! Sums two arrays.
	CSparseBandedArray<T> operator+(const CSparseBandedArray<T>& array);

	//! Subtracts two arrays.
	CSparseBandedArray<T> operator-(const CSparseBandedArray<T>& array);

	//! Multiplies the object with a dense array from the right.
	CDenseArray<T> operator*(const CDenseArray<T>& array);

	//! Multiplies the object with a sparse array from the right.
	CSparseBandedArray<T> operator*(CSparseBandedArray<T>& array);

	//! Solves linear system associated with this matrix.
	virtual void Solve(CDenseArray<T>& x, const CDenseArray<T>& b) {};

	//! Deletes an element.
	void Delete(size_t i, size_t j);

	//! Deletes a row.
	void DeleteRow(size_t i);

	//! Deletes a column.
	void DeleteCol(size_t i);

	//! Counts the number of non-zeros.
	size_t Nonzeros();

	//! Returns the number of bands.
	size_t NoBands() { return m_data.size(); };

	//! Saves the matrix in matrix market format.
	bool SaveToFile(const char* filename);

protected:

	size_t m_nrows;														//!< number of rows
	size_t m_ncols;														//!< number of cols
	bool m_transpose;													//!< transpose flag
	std::map<int,std::map<size_t,T> > m_data;							//!< container that holds the array data

	//! Computes the band index.
	int Band(size_t i, size_t j) { return (int)j - (int)i; };

	//! Computes the location in the band.
	size_t BandIndex(size_t i, size_t j) { 	return j<=i ? j : i; };

	//! Computes the row from band/diagonal index representation.
	size_t Row(int b, size_t d) { return b<=0 ? size_t((int)d-b) : d; };

	//! Computes the row from band/diagonal index representation.
	size_t Col(int b, size_t d) { return b<=0 ? d : size_t((int)d+b); };

	//! Swaps the two input indices.
	void Transpose(size_t& i, size_t& j);

};



/*! \brief sparse square diagonal matrix
 *
 *
 *
 */
template<class T>
class CSparseDiagonalArray: public CSparseBandedArray<T> {

public:

	//! Constructor.
	CSparseDiagonalArray();

	//! Constructor.
	CSparseDiagonalArray(size_t nrows, size_t ncols);

	//! Copy/casting constructor.
	CSparseDiagonalArray(CSparseBandedArray<T>& array);

	//! Copy/casting constructor.
	CSparseDiagonalArray(CSparseArray<T>& array);

	//! Non-destructive element access.
	T Get(size_t i) { return CSparseBandedArray<T>::Get(i,i); };

	//! \copydoc CSparseBandedArray::Set(size_t,size_t,T)
	void Set(size_t i, size_t j, T v);

	//! Sets a matrix element to a given value.
	void Set(size_t i, T v) { Set(i,i,v); };

	//! \copydoc CSparseBandedArray::operator()(size_t,size_t)
	T& operator()(size_t i, size_t j);

	//! \copydoc CSparseBandedArray::Solve(CDenseArray<T>&,const CDenseArray<T>&)
	void Solve(CDenseArray<T>& x, const CDenseArray<T>& b);

	//! Method stump (transposition does nothing to square diagonal matrices).
	virtual void Transpose() {};

	//! Computes the inverse in-place.
	void Invert();

protected:

	using CSparseBandedArray<T>::m_data;
	using CSparseBandedArray<T>::m_nrows;
	using CSparseBandedArray<T>::m_ncols;
//	using CSparseBandedArray<T>::m_transpose;

};

/*! \brief sparse upper-triangular matrix
 *
 * \todo Maybe one triangular banded class is enough + possibility to transpose?
 * \todo Check whether banded storage scheme is really faster for back-substitution.
 *
 */
template<class T>
class CSparseUpperTriangularArray: public CSparseBandedArray<T> {

public:

	//! Constructor.
	CSparseUpperTriangularArray();

	//! Constructor.
	CSparseUpperTriangularArray(size_t nrows, size_t ncols);

	//! Copy/casting constructor.
	CSparseUpperTriangularArray(CSparseBandedArray<T>& array);

	//! Copy/casting constructor.
	CSparseUpperTriangularArray(CSparseArray<T>& array);

	//! \copydoc CSparseBandedArray::Get(size_t,size_t)
	T Get(size_t i, size_t j);

	//! \copydoc CSparseBandedArray::Set(size_t,size_t,T)
	void Set(size_t i, size_t j, T v);

	//! \copydoc CSparseBandedArray::operator()(size_t,size_t)
	T& operator()(size_t i, size_t j);

	//! \copydoc CSparseBandedArray::Solve(CDenseArray<T>&,const CDenseArray<T>&)
	void Solve(CDenseArray<T>& x, const CDenseArray<T>& b);

	//! Method stump (no transposition allowed for triangular matrices).
	virtual void Transpose() {};

protected:

	using CSparseBandedArray<T>::m_data;
	using CSparseBandedArray<T>::m_nrows;
	using CSparseBandedArray<T>::m_ncols;
	using CSparseBandedArray<T>::m_transpose;

};


/*! \brief sparse lower-triangular matrix
 *
 *
 *
 */
template<class T>
class CSparseLowerTriangularArray: public CSparseBandedArray<T> {

public:

	//! Constructor.
	CSparseLowerTriangularArray();

	//! Constructor.
	CSparseLowerTriangularArray(size_t nrows, size_t ncols);

	//! Copy/casting constructor.
	CSparseLowerTriangularArray(CSparseBandedArray<T>& array);

	//! Copy/casting constructor.
	CSparseLowerTriangularArray(CSparseArray<T>& array);

	//! \copydoc CSparseBandedArray::Get(size_t,size_t)
	T Get(size_t i, size_t j);

	//! \copydoc CSparseBandedArray::Set(size_t,size_t,T)
	void Set(size_t i, size_t j, T v);

	//! \copydoc CSparseBandedArray::operator()(size_t,size_t)
	T& operator()(size_t i, size_t j);

	//! \copydoc CSparseBandedArray::Solve(CDenseArray<T>&,const CDenseArray<T>&)
	void Solve(CDenseArray<T>& x, const CDenseArray<T>& b);

protected:

	using CSparseBandedArray<T>::m_data;
	using CSparseBandedArray<T>::m_nrows;
	using CSparseBandedArray<T>::m_ncols;
	using CSparseBandedArray<T>::m_transpose;


};



}

#endif /* SARRAY_H_ */