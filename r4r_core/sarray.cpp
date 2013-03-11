/*
 * sarray.cpp
 *
 *  Created on: Apr 10, 2012
 *      Author: jbalzer
 */




#include "sarray.h"
#include "darray.h"

#include <string.h>
#include <vector>
#include <math.h>
#include <stdio.h>
#include <assert.h>
#include <algorithm>
#include <fstream>


#ifdef _OPENMP
#include <omp.h>
#endif

using namespace std;

namespace R4R {

template <class T>
CSparseArray<T>::CSparseArray():
	m_nrows(0),
	m_ncols(0),
	m_transpose(false),
	m_data() {


}


template <class T>
CSparseArray<T>::CSparseArray(size_t nrows, size_t ncols):
	m_nrows(nrows),
	m_ncols(ncols),
	m_transpose(false),
	m_data() {

}

template<class U>
ostream& operator << (ostream& os, CSparseArray<U>& x) {

	typename map<size_t,map<size_t,U> >::iterator it_row;
	typename map<size_t,U>::iterator it_col;

	it_row = x.m_data.begin();

	while(it_row != x.m_data.end()) {

		it_col = it_row->second.begin();

		while(it_col!=it_row->second.end()) {

			if(x.m_transpose)
				os << "[" << (int)it_col->first << "," << (int)it_row->first << "] " << (float)it_col->second << endl;
			else
				os << "[" << (int)it_row->first << "," << (int)it_col->first << "] " << (float)it_col->second << endl;

			it_col++;

		}

		os << endl;

		it_row++;

	}

	return os;

};

template <class T>
T CSparseArray<T>::Norm2() {

	T sum = 0;

	typename map<size_t,map<size_t,T> >::iterator it_row;
	typename map<size_t,T>::iterator it_col;

	it_row = m_data.begin();

	while(it_row != m_data.end()) {

		it_col = it_row->second.begin();

		while(it_col!=it_row->second.end()) {

			sum += (it_col->second)*(it_col->second);

			it_col++;

		}

		it_row++;

	}

	return sqrt(sum);

}

template <class T>
void CSparseArray<T>::Transpose() {

	size_t temp = m_nrows;

	m_nrows = m_ncols;
	m_ncols = temp;

	m_transpose = !m_transpose;

}

template <class T>
T& CSparseArray<T>::operator()(size_t i, size_t j) {

	assert(i>=0 && i<m_nrows && j>=0 && j<m_ncols);

	if(m_transpose)
		Transpose(i,j);

	return m_data[i][j];

}


template <class T>
T CSparseArray<T>::Get(size_t i, size_t j) {

	if(m_transpose)
		Transpose(i,j);

	assert(i>=0 && i<m_nrows && j>=0 && j<m_ncols);

	typename map<size_t,map<size_t,T> >::iterator it_row;

	it_row = m_data.find(i);

	if(it_row == m_data.end())
		return (T)0;

	typename map<size_t,T>::iterator it_col;

	it_col = it_row->second.find(j);

	if(it_col == it_row->second.end())
		return (T)0;

	return it_col->second;

}

template <class T>
std::map<size_t,T> CSparseArray<T>::GetRow(size_t i) {

	assert(i>=0 && i<m_nrows);

	map<size_t,T> row;

	if(!m_transpose) {

		if(m_data.find(i)!=m_data.end())
			row = m_data[i];

	}
	else {

		typename map<size_t,map<size_t,T> >::iterator it_col = m_data.begin();
		typename map<size_t,T>::iterator it_row;

		while(it_col!=m_data.end()) {

			it_row = it_col->second.begin();

			while(it_row!=it_col->second.end()) {


				if(it_col->second.find(i)!=it_col->second.end())
					row.insert(pair<size_t,T>(it_col->first,it_row->second));

				it_row++;

			}

			it_col++;

		}

	}

	return row;

}


template <class T>
void CSparseArray<T>::Set(size_t i, size_t j, T v) {

	if(m_transpose)
		Transpose(i,j);

	assert(i>=0 && i<m_nrows && j>=0 && j<m_ncols);

	if(v==0){	// if the new value is zero, delete entry

		typename map<size_t,map<size_t,T> >::iterator it_row;
		typename map<size_t,T>::iterator it_col;

		it_row = m_data.find(i);

		if(it_row == m_data.end())
			return;

		it_col = it_row->second.find(j);

		if(it_col == it_row->second.end())
			return;

		it_row->second.erase(it_col);

		if(it_row->second.empty())
			m_data.erase(it_row);

	}
	else
		m_data[i][j] = v;

}

template <class T>
CSparseArray<T> CSparseArray<T>::operator*(const T& scalar) {

	CSparseArray<T> result = *this;

	typename map<size_t,map<size_t,T> >::iterator it_row;
	typename map<size_t,T>::iterator it_col;

	it_row = result.m_data.begin();

	while(it_row != result.m_data.end()) {

		it_col = it_row->second.begin();

		while(it_col!=it_row->second.end()) {

			it_col->second *= scalar;

			it_col++;

		}

		it_row++;

	}

	return result;

}


template <class T>
CSparseArray<T> CSparseArray<T>::operator+(const CSparseArray& array) {

	assert(m_nrows==array.m_nrows && m_ncols==array.m_ncols);

	CSparseArray<T> result = array;		// make a copy of the input

	typename map<size_t,map<size_t,T> >::iterator it_row;
	typename map<size_t,T>::iterator it_col;

	it_row = m_data.begin();

	while(it_row != m_data.end()) {

		it_col = it_row->second.begin();

		while(it_col!=it_row->second.end()) {

			result(it_row->first,it_col->first) += it_col->second;

			it_col++;

		}

		it_row++;

	}

	return result;

}

template <class T>
CSparseArray<T> CSparseArray<T>::operator-(const CSparseArray& array) {

	assert(m_nrows==array.m_nrows && m_ncols==array.m_ncols);

	CSparseArray<T> result = array;		// make a copy of the input

	typename map<size_t,map<size_t,T> >::iterator it_row;
	typename map<size_t,T>::iterator it_col;

	it_row = m_data.begin();

	while(it_row != m_data.end()) {

		it_col = it_row->second.begin();

		while(it_col!=it_row->second.end()) {

			result(it_row->first,it_col->first) -= it_col->second;

			it_col++;

		}

		it_row++;

	}

	return result;

}

template <class T>
T CSparseArray<T>::InnerProduct(CSparseArray<T>& x, CSparseArray<T>& y) {

	assert(x.m_nrows==y.m_nrows && x.m_ncols==y.m_ncols);

	T sum = 0;

	typename map<size_t,map<size_t,T> >::iterator it_row;
	typename map<size_t,T>::iterator it_col;

	it_row = x.m_data.begin();

	while(it_row!=x.m_data.end()) {

		map<size_t,T> rowx = x.GetRow(it_row->first);
		map<size_t,T> rowy = y.GetRow(it_row->first);

		for(it_col=rowx.begin(); it_col!=rowx.end(); it_col++)
			sum+= it_col->second*rowy[it_col->first];

		it_row++;

	}

	return sum;

}

template <class T>
template<class Matrix> Matrix CSparseArray<T>::operator*(Matrix& array) {

#ifdef _OPENMP
	assert(m_ncols==array.NRows());

	Matrix result(m_nrows,array.NCols());;

	if(!m_transpose) {

		typename map<size_t,T>::iterator it_col;

#pragma omp parallel for private(it_col), shared(result)
		for(size_t i=0; i<m_nrows; i++) {

			for(size_t j=0; j<array.NCols(); j++) {

				if(m_data[i].size()>0) {

					T sum = 0;

					for(it_col=m_data[i].begin(); it_col!=m_data[i].end(); it_col++)
						sum+= (it_col->second)*array.Get(it_col->first,j);

					if(sum!=0)
						result(i,j) = sum;

				}

			}

		}

	}
	else {

		typename map<size_t,T>::iterator it_row;

		for(size_t j=0; j<array.NCols(); j++) {

#pragma omp parallel for private(it_row), shared(result)
			for(size_t i=0; i<m_ncols; i++) {


				if(m_data[i].size()>0) {

					for(it_row=m_data[i].begin(); it_row!=m_data[i].end(); it_row++) {




#pragma omp critical
						{
						T val = it_row->second*array.Get(i,j);

						// check this to avoid fill-in
						if(val!=0)
							result(it_row->first,j) += val;

						}

					}

				}

			}

		}

	}

	return result;
#else

	assert(m_ncols==array.NRows());

	Matrix result(m_nrows,array.NCols());;

	if(!m_transpose) {

		typename map<size_t,map<size_t,T> >::iterator it_row;
		typename map<size_t,T>::iterator it_col;

		for(it_row=m_data.begin(); it_row!=m_data.end(); it_row++) {

			for(size_t j=0; j<array.NCols(); j++) {

				T sum = 0;

				for(it_col=it_row->second.begin(); it_col!=it_row->second.end(); it_col++)
					sum+= (it_col->second)*array.Get(it_col->first,j);

				if(sum!=0)
					result(it_row->first,j) = sum;

			}

		}

	}
	else {

		typename map<size_t,map<size_t,T> >::iterator it_col;
		typename map<size_t,T>::iterator it_row;

		for(size_t j=0; j<array.NCols(); j++) {

				for(it_col=m_data.begin(); it_col!=m_data.end(); it_col++) {

				for(it_row=it_col->second.begin(); it_row!=it_col->second.end(); it_row++) {

					T val = it_row->second*array.Get(it_col->first,j);

					if(val!=0)
						result(it_row->first,j) += val;

				}

			}

		}

	}

	return result;
#endif

}

template <class T>
CDenseVector<T> CSparseArray<T>::operator*(const CDenseVector<T>& vector) {

#ifdef _OPENMP
	assert(m_ncols==vector.NRows());

	CDenseVector<T> result = CDenseVector<T>(m_nrows);

	if(!m_transpose) {

		typename map<size_t,T>::iterator it_col;

#pragma omp parallel for private(it_col), shared(result,vector)
		for(size_t i=0; i<m_nrows; i++) {

			if(m_data[i].size()>0) {

				T sum = 0;

				for(it_col=m_data[i].begin(); it_col!=m_data[i].end(); it_col++)
					sum+= (it_col->second)*vector.Get(it_col->first);

				result(i) = sum;

			}

		}

	}
	else {

		typename map<size_t,T>::iterator it_row;

#pragma omp parallel for private(it_row), shared(result,vector)
		for(size_t i=0; i<m_ncols; i++) {

			if(m_data[i].size()>0) {

				for(it_row=m_data[i].begin(); it_row!=m_data[i].end(); it_row++) {

#pragma omp critical
					result(it_row->first) += it_row->second*vector.Get(i);				// protect this

				}

			}

		}

	}

	return result;
#else

	assert(m_ncols==vector.NRows());

	CDenseVector<T> result = CDenseVector<T>(m_nrows);

	if(!m_transpose) {

		typename map<size_t,map<size_t,T> >::iterator it_row;
		typename map<size_t,T>::iterator it_col;

		for(it_row=m_data.begin(); it_row!=m_data.end(); it_row++) {

			T sum = 0;

			for(it_col=it_row->second.begin(); it_col!=it_row->second.end(); it_col++)
				sum+= (it_col->second)*vector.Get(it_col->first);

			result(it_row->first) = sum;

		}

	}
	else {

		typename map<size_t,map<size_t,T> >::iterator it_col;
		typename map<size_t,T>::iterator it_row;

		for(it_col=m_data.begin(); it_col!=m_data.end(); it_col++) {

			for(it_row=it_col->second.begin(); it_row!=it_col->second.end(); it_row++)
				result(it_row->first) += it_row->second*vector.Get(it_col->first);

		}

	}

	return result;
#endif

}

template <class T>
void CSparseArray<T>::Scale(T scalar) {

	typename map<size_t,map<size_t,T> >::iterator it_row;
	typename map<size_t,T>::iterator it_col;

	it_row = m_data.begin();

	while(it_row != m_data.end()) {

		it_col = it_row->second.begin();

		while(it_col!=it_row->second.end()) {

			it_col->second *= scalar;

			it_col++;

		}

		it_row++;

	}

}


template <class T>
void CSparseArray<T>::Transpose(size_t& i, size_t& j) {

	size_t temp = i;

	i = j;

	j = temp;

}


template <class T>
CSparseArray<T> CSparseArray<T>::Transpose(const CSparseArray<T>& array) {

	CSparseArray<T> result = array;

	result.Transpose();

	return result;

}

template <class T>
CSparseArray<T> CSparseArray<T>::Square(CSparseArray<T>& array) {

	CSparseArray<T> result(array.m_nrows,array.m_nrows);

	typename map<size_t,map<size_t,T> >::iterator it_row1;
	typename map<size_t,map<size_t,T> >::iterator it_row2;
	typename map<size_t,T>::iterator it_el;

	// multiply each row with itself if there is enough overlap
	for(it_row1 = array.m_data.begin(); it_row1 != array.m_data.end(); it_row1++) {

		for(it_row2 = array.m_data.begin(); it_row2 != array.m_data.end(); it_row2++) {

			// check if there is overlap
			if(it_row1->second.rbegin()->first >= it_row2->second.begin()->first) {

				T sum = 0;

				for(it_el = it_row1->second.begin(); it_el != it_row1->second.end(); it_el++)
					sum += it_el->second*array.Get(it_row2->first,it_el->first);   // no: maybe access over it_row2 but make sure to avoid fill-in

				// check this to avoid fill-in
				if(sum!=0)
					result(it_row1->first,it_row2->first) = sum;

			}

		}

	}

	return result;

}




template <class T>
size_t CSparseArray<T>::Nonzeros() {

	typename map<size_t,map<size_t,T> >::iterator it_row;

	size_t nnz = 0;

	it_row = m_data.begin();

	while(it_row!=m_data.end()){

		nnz += it_row->second.size();

		it_row++;

	}

	return nnz;

}

template <class T>
bool CSparseArray<T>::Symmetric() {

	bool result = true;

	typename map<size_t,map<size_t,T> >::iterator it_row;
	typename map<size_t,T>::iterator it_col;

	it_row = m_data.begin();

	while(it_row!=m_data.end()){

		it_col = it_row->second.begin();

		while(it_col->first<it_row->first) {

			if(it_col->second!=Get(it_col->first,it_row->first)) {

				return false;

			}

			it_col++;

		}

		it_row++;

	}

	return result;

}


template <class T>
void CSparseArray<T>::DeleteRow(size_t i) {

	typename map<size_t,map<size_t,T> >::iterator it_row;

	it_row = m_data.find(i);

	if(it_row!=m_data.end())
		m_data.erase(it_row);

}

template <class T>
void CSparseArray<T>::DeleteColumn(size_t j) {

	typename map<size_t,map<size_t,T> >::iterator it_row;
	typename map<size_t,T>::iterator it_col;

	it_row = m_data.begin();

	while(it_row!=m_data.end()){

		it_col = it_row->second.find(j);

		if(it_col!=it_row->second.end())
			it_row->second.erase(it_col);

		it_row++;

	}

}

template <class T>
void CSparseArray<T>::GetCSR(vector<size_t>& nz, vector<size_t>& j, vector<T>& v, bool ibase) {

	typename map<size_t,map<size_t,T> >::iterator it_row = m_data.begin();
	typename map<size_t,T>::iterator it_col;

	size_t nnz = ibase;

	while(it_row!=m_data.end()){

		nz.push_back(nnz);

		for (it_col=it_row->second.begin(); it_col!=it_row->second.end(); it_col++) {

			j.push_back(it_col->first+ibase);

			v.push_back((T)(it_col->second));

			nnz++;

		}

		it_row++;

	}

	nz.push_back(nnz);

}

template <class T>
void CSparseArray<T>::GetCOO(std::vector<size_t>& i, std::vector<size_t>& j, std::vector<T>& v, bool ibase) {

	typename map<size_t,map<size_t,T> >::iterator it_row = m_data.begin();
	typename map<size_t,T>::iterator it_col;

	while(it_row!=m_data.end()){

		for (it_col=it_row->second.begin(); it_col!=it_row->second.end(); it_col++) {

			i.push_back(it_row->first+ibase);
			j.push_back(it_col->first+ibase);
			v.push_back((T)(it_col->second));

		}

		it_row++;

	}

}

template <class T>
bool CSparseArray<T>::SaveToFile(const char* filename) {

	ofstream out(filename);

	if(!out) {

		cout << "ERROR: Could not open file.\n";
		return 1;

	 }

	// write header
	out << "\%\%MatrixMarket matrix coordinate real general" << endl;
	out << m_nrows << " " << m_ncols << " " << Nonzeros() << endl;

	typename map<size_t,map<size_t,T> >::iterator it_row;
	typename map<size_t,T>::iterator it_col;

	it_row = m_data.begin();

	while(it_row != m_data.end()) {

		it_col = it_row->second.begin();

		while(it_col!=it_row->second.end()) {

			if(m_transpose)
				out << (int)it_col->first << " " << (int)it_row->first << " " << (float)it_col->second << endl;
			else
				out << (int)it_row->first << " " << (int)it_col->first << " " << (float)it_col->second << endl;

			it_col++;

		}

		it_row++;

	}

	out.close();

	return 0;

}

template <class T>
void CSparseArray<T>::Rand(size_t nnz) {

	for(size_t k=0; k<nnz; k++) {

		size_t i = (size_t)floor(((double)rand()/(double)RAND_MAX)*(double)m_nrows);
		size_t j = (size_t)floor(((double)rand()/(double)RAND_MAX)*(double)m_ncols);
		this->operator ()(i,j) = (T)((T)rand()/(T)RAND_MAX);

	}

}


template class CSparseArray<float>;
template class CSparseArray<double>;
template class CSparseArray<int>;
template CDenseArray<double> CSparseArray<double>::operator *(CDenseArray<double>& x);
template CDenseVector<double> CSparseArray<double>::operator *(CDenseVector<double>& x);
template CSparseArray<double> CSparseArray<double>::operator *(CSparseArray<double>& x);


template ostream& operator<< (ostream& os, CSparseArray<double>& x);

template <class T>
CSparseBandedArray<T>::CSparseBandedArray():
	m_nrows(0),
	m_ncols(0),
	m_transpose(false),
	m_data() {

}

template <class T>
CSparseBandedArray<T>::CSparseBandedArray(size_t nrows, size_t ncols):
	m_nrows(nrows),
	m_ncols(ncols),
	m_transpose(false),
	m_data() {

}

template <class T>
void CSparseBandedArray<T>::Transpose(size_t& i, size_t& j) {

	size_t temp = i;

	i = j;

	j = temp;

}

template <class T>
CSparseBandedArray<T> CSparseBandedArray<T>::Transpose(const CSparseBandedArray<T>& array) {

	CSparseBandedArray<T> result = array;

	result.Transpose();

	return result;

}

template <class T>
void CSparseBandedArray<T>::Delete(size_t i, size_t j) {

	assert(i>=0 && i<m_nrows && j>=0 && j<m_ncols);

	if(m_transpose)
		Transpose(i,j);

	int b = Band(i,j);
	size_t d = BandIndex(i,j);

	// delete the value if it exists
	m_data[b].erase(d);

	// remove the band, if it is now empty
	if(m_data[b].empty())
		m_data.erase(b);

}

template <class T>
void CSparseBandedArray<T>::DeleteRow(size_t i) {

	assert(i>=0 && i<m_nrows);

	for(size_t j=0; j<m_ncols; j++)
		Delete(i,j);

}

template <class T>
void CSparseBandedArray<T>::DeleteCol(size_t j) {

	assert(j>=0 && j<m_ncols);

	for(size_t i=0; i<m_nrows; i++)
		Delete(i,j);

}

template <class T>
void CSparseBandedArray<T>::Print() {

	typename map<int,map<size_t,T> >::iterator it_b;
	typename map<size_t,T>::iterator it_d;

	it_b = m_data.begin();

	while(it_b != m_data.end()) {

		it_d = it_b->second.begin();

		while(it_d!=it_b->second.end()) {

			if(m_transpose)
				printf("[%d,%d] %.4f\n",(int)Row(-it_b->first,it_d->first),(int)Col(-it_b->first,it_d->first),(float)it_d->second);
			else
				printf("[%d,%d] %.4f\n",(int)Row(it_b->first,it_d->first),(int)Col(it_b->first,it_d->first),(float)it_d->second);

			it_d++;

		}

		printf("\n");

		it_b++;

	}

}

template <class T>
T CSparseBandedArray<T>::Get(size_t i, size_t j) {

	if(m_transpose)
		Transpose(i,j);

	assert(i>=0 && i<m_nrows && j>=0 && j<m_ncols);

	// convert index
	int b = Band(i,j);
	size_t d = BandIndex(i,j);

	typename map<int,map<size_t,T> >::iterator it_b;

	it_b = m_data.find(b);

	if(it_b == m_data.end())
		return (T)0;

	typename map<size_t,T>::iterator it_d;

	it_d = it_b->second.find(d);

	if(it_d == it_b->second.end())
		return (T)0;

	return it_d->second;

}

template <class T>
map<int,map<size_t,T> > CSparseBandedArray<T>::GetBands(int lower, int upper) {

	assert(lower<=upper);

	if(m_transpose) {

		int temp = lower;
		lower = -upper;
		upper = -temp;

	}

	map<int,map<size_t,T> > result;

	typename map<int,map<size_t,T> >::iterator it_b = m_data.find(lower);

	if(it_b==m_data.end())
		it_b = m_data.begin();

	while(it_b->first <=upper && it_b!=m_data.end()) {

		if(it_b->first>=lower) {

			if(m_transpose)
				result.insert(pair<int,std::map<size_t,T> >(-it_b->first,it_b->second));
			else
				result.insert(pair<int,std::map<size_t,T> >(it_b->first,it_b->second));

		}

		it_b++;
	}

	return result;

}


template <class T>
void CSparseBandedArray<T>::Set(size_t i, size_t j, T v) {

	if(m_transpose)
		Transpose(i,j);

	assert(i>=0 && i<m_nrows && j>=0 && j<m_ncols);

	// convert index
	int b = Band(i,j);
	size_t d = BandIndex(i,j);

	if(v==0){	// if the new value is zero, delete entry

		typename map<int,map<size_t,T> >::iterator it_b;
		typename map<size_t,T>::iterator it_d;

		it_b = m_data.find(b);

		if(it_b == m_data.end())
			return;

		it_d = it_b->second.find(d);

		if(it_d == it_b->second.end())
			return;

		it_b->second.erase(it_d);

		if(it_b->second.empty())
			m_data.erase(it_b);

	}
	else
		m_data[b][d] = v;

}



template <class T>
T& CSparseBandedArray<T>::operator()(size_t i, size_t j) {

	assert(i>=0 && i<m_nrows && j>=0 && j<m_ncols);

	if(m_transpose)
		Transpose(i,j);

	int b = Band(i,j);
	size_t d = BandIndex(i,j);

	return m_data[b][d];

}

template <class T>
T CSparseBandedArray<T>::Norm2() {

	T sum = 0;

	typename map<int,map<size_t,T> >::iterator it_b;
	typename map<size_t,T>::iterator it_d;

	it_b = m_data.begin();

	while(it_b != m_data.end()) {

		it_d = it_b->second.begin();

		while(it_d!=it_b->second.end()) {

			sum += (it_d->second)*(it_d->second);

			it_d++;

		}

		it_b++;

	}

	return sqrt(sum);

}

template <class T>
void CSparseBandedArray<T>::Transpose() {

	size_t temp = m_nrows;

	m_nrows = m_ncols;
	m_ncols = temp;

	m_transpose = !m_transpose;

}

template <class T>
CSparseBandedArray<T> CSparseBandedArray<T>::operator+(const CSparseBandedArray& array) {

	assert(m_nrows==array.m_nrows && m_ncols==array.m_ncols);

	CSparseBandedArray<T> result = array;		// make a copy of the input

	typename map<int,map<size_t,T> >::iterator it_b;
	typename map<size_t,T>::iterator it_d;

	it_b = m_data.begin();

	while(it_b != m_data.end()) {

		it_d = it_b->second.begin();

		while(it_d!=it_b->second.end()) {

			if(result.m_transpose)
				result.m_data[-it_b->first][it_d->first] += it_d->second;
			else
				result.m_data[it_b->first][it_d->first] += it_d->second;

			it_d++;

		}

		it_b++;

	}

	return result;

}


template <class T>
CSparseBandedArray<T> CSparseBandedArray<T>::operator-(const CSparseBandedArray& array) {

	assert(m_nrows==array.m_nrows && m_ncols==array.m_ncols);

	CSparseBandedArray<T> result = array;		// make a copy of the input

	typename map<int,map<size_t,T> >::iterator it_b;
	typename map<size_t,T>::iterator it_d;

	it_b = m_data.begin();

	while(it_b != m_data.end()) {

		it_d = it_b->second.begin();

		while(it_d!=it_b->second.end()) {

			if(result.m_transpose)
				result.m_data[-it_b->first][it_d->first] -= it_d->second;
			else
				result.m_data[it_b->first][it_d->first] -= it_d->second;

			it_d++;

		}

		it_b++;

	}

	return result;

}

template <class T>
CDenseArray<T> CSparseBandedArray<T>::operator*(const CDenseArray<T>& array) {

	assert(m_ncols==array.NRows());

	CDenseArray<T> result = CDenseArray<T>(m_nrows,array.NCols());;

	typename map<int,map<size_t,T> >::iterator it_b;
	typename map<size_t,T>::iterator it_d;

	// for all columns
	for(size_t k=0; k<array.NCols(); k++) {

		for(it_b=m_data.begin(); it_b!=m_data.end(); it_b++) {

			for(it_d=it_b->second.begin(); it_d!=it_b->second.end(); it_d++) {

				size_t i = Row(it_b->first,it_d->first);
				size_t j = Col(it_b->first,it_d->first);

				if(m_transpose)
					Transpose(i,j);

				result(i,k) += it_d->second*array.Get(j,k);

			}

		}

	}

	return result;

}

template <class T>
CSparseBandedArray<T> CSparseBandedArray<T>::operator*(CSparseBandedArray<T>& array) {
	assert(m_ncols==array.NRows());

	CSparseBandedArray<T> result = CSparseBandedArray<T>(m_nrows,array.NCols());;

	typename map<int,map<size_t,T> >::iterator it_b;
	typename map<size_t,T>::iterator it_d;

	// for all columns
	for(size_t k=0; k<array.NCols(); k++) {

		for(it_b=m_data.begin(); it_b!=m_data.end(); it_b++) {

			for(it_d=it_b->second.begin(); it_d!=it_b->second.end(); it_d++) {

				size_t i = Row(it_b->first,it_d->first);
				size_t j = Col(it_b->first,it_d->first);

				if(m_transpose)
					Transpose(i,j);

				T temp = it_d->second*array.Get(j,k);

				if(temp!=0)
					result(i,k) += temp;

			}

		}

	}

	return result;

}


template <class T>
void CSparseBandedArray<T>::Scale(T scalar) {

	typename map<int,map<size_t,T> >::iterator it_b;
	typename map<size_t,T>::iterator it_d;

	it_b = m_data.begin();

	while(it_b != m_data.end()) {

		it_d = it_b->second.begin();

		while(it_d!=it_b->second.end()) {

			it_d->second *= scalar;

			it_d++;

		}

		it_b++;

	}

}

template <class T>
void CSparseBandedArray<T>::AddDiagonal(const T scalar) {

	for(size_t i=0; i<min(m_nrows,m_ncols); i++)
		this->operator ()(i,i) += scalar;

}

template <class T>
void CSparseBandedArray<T>::ScaleDiagonal(T scalar) {

	typename map<int,map<size_t,T> >::iterator it_b;
	typename map<size_t,T>::iterator it_d;

	it_b = m_data.find(0);

	if(it_b==m_data.end())
		return;

	it_d = it_b->second.begin();

	while(it_d!=it_b->second.end()) {

		it_d->second *= scalar;

		it_d++;

	}

}


template <class T>
size_t CSparseBandedArray<T>::Nonzeros() {

	typename map<int,map<size_t,T> >::iterator it_b;

	size_t nnz = 0;

	it_b = m_data.begin();

	while(it_b != m_data.end()) {

		nnz += it_b->second.size();

		it_b++;

	}

	return nnz;

}

template <class T>
bool CSparseBandedArray<T>::SaveToFile(const char* filename) {

	ofstream out(filename);

	if(!out) {

		cout << "ERROR: Could not open file.\n";
		return 1;

	 }

	// write header
	out << "\%\%MatrixMarket matrix coordinate real general" << endl;
	out << m_nrows << " " << m_ncols << " " << Nonzeros() << endl;

	// write data
	typename map<int,map<size_t,T> >::iterator it_b;
	typename map<size_t,T>::iterator it_d;

	it_b = m_data.begin();

	while(it_b != m_data.end()) {

		it_d = it_b->second.begin();

		while(it_d!=it_b->second.end()) {

			size_t i = Row(it_b->first,it_d->first);
			size_t j = Col(it_b->first,it_d->first);

			if(m_transpose)
				Transpose(i,j);

			out << i << " " << j << " " << it_d->second;

			it_d++;

		}

		it_b++;

	}

	out.close();

	return 0;

}


template class CSparseBandedArray<float>;
template class CSparseBandedArray<double>;

template <class T>
CSparseDiagonalArray<T>::CSparseDiagonalArray():
	CSparseBandedArray<T>() {


}

template <class T>
CSparseDiagonalArray<T>::CSparseDiagonalArray(size_t nrows, size_t ncols):
	CSparseBandedArray<T>(min(nrows,ncols),min(nrows,ncols)) {

}

template <class T>
CSparseDiagonalArray<T>::CSparseDiagonalArray(CSparseBandedArray<T>& array):
	CSparseBandedArray<T>(min(array.NRows(),array.NCols()),min(array.NRows(),array.NCols())) {

	typename map<int,map<size_t,T> >::iterator it_b;
	typename map<size_t,T>::iterator it_d;

	m_data = array.GetBands(0,0);

};


template <class T>
CSparseDiagonalArray<T>::CSparseDiagonalArray(CSparseArray<T>& array):
	CSparseBandedArray<T>(min(array.NRows(),array.NCols()),min(array.NRows(),array.NCols())) {

	for(size_t i=0; i<min(m_nrows,m_ncols); i++) {

		if(array.Get(i,i)!=0) {

			this->operator ()(i,i) = array.Get(i,i);

		}

	}

}

template <class T>
void CSparseDiagonalArray<T>::Set(size_t i, size_t j, T v) {

	assert(i==j);

	CSparseBandedArray<T>::Set(i,j,v);

}

template <class T>
T& CSparseDiagonalArray<T>::operator()(size_t i, size_t j) {

	assert(i==j);

	return CSparseBandedArray<T>::operator()(i,j);

}

template <class T>
void CSparseDiagonalArray<T>::Solve(CDenseArray<T>& x, const CDenseArray<T>& b) {

	assert(b.NRows()==m_nrows && x.NRows()==m_nrows);

	typename map<int,map<size_t,T> >::iterator it_b;
	typename map<size_t,T>::iterator it_d;

	it_b = m_data.find(0);

	if(it_b->second.size()!= b.NRows()) {

		cout << "ERROR: Matrix is rank-deficient." << endl;
		return;

	}

	it_d = it_b->second.begin();

	while(it_d!=it_b->second.end()) {

		for(size_t j=0; j<b.NCols(); j++)
			x(it_d->first,j) = b.Get(it_d->first,j)/it_d->second;

		it_d++;

	}

}


template <class T>
void CSparseDiagonalArray<T>::Invert() {

	typename map<int,map<size_t,T> >::iterator it_b;
	typename map<size_t,T>::iterator it_d;

	it_b = m_data.find(0);

	if(it_b->second.size()!=m_nrows) {

		printf("ERROR: Matrix is rank-deficient.\n");
		return;

	}

	it_d = it_b->second.begin();

	while(it_d!=it_b->second.end()) {

		it_d->second = 1/it_d->second;

		it_d++;

	}

}


template class CSparseDiagonalArray<float>;
template class CSparseDiagonalArray<double>;


template <class T>
CSparseUpperTriangularArray<T>::CSparseUpperTriangularArray():
	CSparseBandedArray<T>() {

}

template <class T>
CSparseUpperTriangularArray<T>::CSparseUpperTriangularArray(size_t nrows, size_t ncols):
	CSparseBandedArray<T>(min(nrows,ncols),min(nrows,ncols)) {

}

template <class T>
CSparseUpperTriangularArray<T>::CSparseUpperTriangularArray(CSparseBandedArray<T>& array):
	CSparseBandedArray<T>(min(array.NRows(),array.NCols()),min(array.NRows(),array.NCols())) {

	typename map<int,map<size_t,T> >::iterator it_b;
	typename map<size_t,T>::iterator it_d;

	m_data = array.GetBands(0,array.NRows()+array.NCols());

};

template <class T>
CSparseUpperTriangularArray<T>::CSparseUpperTriangularArray(CSparseArray<T>& array):
	CSparseBandedArray<T>(min(array.NRows(),array.NCols()),min(array.NRows(),array.NCols())) {

	// iterate through incoming array to avoid zeros
	typename map<size_t,map<size_t,T> >::iterator it_row;
	typename map<size_t,T>::iterator it_col;

	it_row = array.m_data.begin();

	while(it_row!=array.m_data.end()){

		it_col = it_row->second.begin();

		while(it_col!=it_row->second.end()) {

			if(!array.m_transpose && it_row->first<=it_col->first)
				this->operator ()(it_row->first,it_col->first) = it_col->second;
			else if(array.m_transpose && it_row->first>=it_col->first)
				this->operator ()(it_col->first,it_row->first) = it_col->second;

			it_col++;

		}

		it_row++;

	}

}



template <class T>
T CSparseUpperTriangularArray<T>::Get(size_t i, size_t j) {

	return j>=i ? CSparseBandedArray<T>::Get(i,j) : 0;

}


template <class T>
void CSparseUpperTriangularArray<T>::Set(size_t i, size_t j, T v) {

	assert(j>=i);

	CSparseBandedArray<T>::Set(i,j,v);

}

template <class T>
T& CSparseUpperTriangularArray<T>::operator()(size_t i, size_t j) {

	assert(j>=i);

	return CSparseBandedArray<T>::operator()(i,j);

}

template <class T>
void CSparseUpperTriangularArray<T>::Solve(CDenseArray<T>& x, const CDenseArray<T>& b) {

	assert(m_nrows>=m_ncols && b.NRows()==m_nrows && x.NRows()==m_ncols);

	typename map<int,map<size_t,T> >::iterator it_diagonal = m_data.find(0);

	if(it_diagonal->second.size()!= b.NRows()) {

		printf("ERROR: Matrix is rank-deficient.\n");
		return;

	}

	map<int,typename map<size_t,T>::reverse_iterator> bands;
	typename map<int,map<size_t,T> >::iterator it_b;

	// increment to start adding everyting above diagonal
	it_diagonal++;

	// init iterators to the beginning of all bands
	for(it_b=it_diagonal; it_b!=m_data.end();it_b++)
		bands.insert(pair<int,typename map<size_t,T>::reverse_iterator >(it_b->first,it_b->second.rbegin()));

	// set iterator back to diagonal
	it_diagonal--;

	typename map<size_t,T>::reverse_iterator it_d = it_diagonal->second.rbegin();
	typename map<int,typename map<size_t,T>::reverse_iterator >::iterator it_bands;

	// one accumulator for each column of b
	T* sums = new T[b.NCols()];

	// iterate through main diagonal
	while(it_d!=it_diagonal->second.rend()) {

		for(size_t bc=0; bc<b.NCols(); bc++)
			sums[bc] = b.Get(it_d->first,bc);

		for(it_bands=bands.begin(); it_bands!=bands.end(); it_bands++) {

			// go through all band iterators and compute their row/column representation
			size_t i = CSparseBandedArray<T>::Row(it_bands->first,it_bands->second->first);
			size_t j = CSparseBandedArray<T>::Col(it_bands->first,it_bands->second->first);


			// check whether the band contributes to the current row
			if(i==it_d->first && j>it_d->first) {

				for(size_t bc=0; bc<b.NCols(); bc++)
					sums[bc] -= it_bands->second->second*x(j,bc);

				// increment the iterator in the band
				it_bands->second++;

			}

		}

		for(size_t bc=0; bc<b.NCols(); bc++)
			x(it_d->first,bc) = sums[bc]/it_d->second;


		it_d++;

	}

	delete [] sums;

}


template class CSparseUpperTriangularArray<float>;
template class CSparseUpperTriangularArray<double>;


template <class T>
CSparseLowerTriangularArray<T>::CSparseLowerTriangularArray():
	CSparseBandedArray<T>() {

}

template <class T>
CSparseLowerTriangularArray<T>::CSparseLowerTriangularArray(size_t nrows, size_t ncols):
	CSparseBandedArray<T>(min(nrows,ncols),min(nrows,ncols)) {

}

template <class T>
CSparseLowerTriangularArray<T>::CSparseLowerTriangularArray(CSparseBandedArray<T>& array):
	CSparseBandedArray<T>(min(array.NRows(),array.NCols()),min(array.NRows(),array.NCols())){

	typename map<int,map<size_t,T> >::iterator it_b;
	typename map<size_t,T>::iterator it_d;

	m_data = array.GetBands(-(array.NRows()+array.NCols()),0);

};

template <class T>
CSparseLowerTriangularArray<T>::CSparseLowerTriangularArray(CSparseArray<T>& array):
	CSparseBandedArray<T>(min(array.NRows(),array.NCols()),min(array.NRows(),array.NCols())) {

	typename map<size_t,map<size_t,T> >::iterator it_row;
	typename map<size_t,T>::iterator it_col;

	it_row = array.m_data.begin();

	while(it_row!=array.m_data.end()){

		it_col = it_row->second.begin();

		while(it_col!=it_row->second.end()) {

			if(!array.m_transpose && it_row->first>=it_col->first)
				this->operator ()(it_row->first,it_col->first) = it_col->second;
			else if(array.m_transpose && it_row->first<=it_col->first)
				this->operator ()(it_col->first,it_row->first) = it_col->second;

			it_col++;

		}

		it_row++;

	}

}

template <class T>
T CSparseLowerTriangularArray<T>::Get(size_t i, size_t j) {

	return j<=i ? CSparseBandedArray<T>::Get(i,j) : 0;

}


template <class T>
void CSparseLowerTriangularArray<T>::Set(size_t i, size_t j, T v) {

	assert(j<=i);

	CSparseBandedArray<T>::Set(i,j,v);

}

template <class T>
T& CSparseLowerTriangularArray<T>::operator()(size_t i, size_t j) {

	assert(j<=i);

	return CSparseBandedArray<T>::operator()(i,j);

}

template <class T>
void CSparseLowerTriangularArray<T>::Solve(CDenseArray<T>& x, const CDenseArray<T>& b) {

	assert(m_nrows>=m_ncols && b.NRows()==m_nrows && x.NRows()==m_ncols);

	typename map<int,map<size_t,T> >::iterator it_diagonal = m_data.find(0);

	if(it_diagonal->second.size()!= b.NRows()) {

		cout << "ERROR: Matrix is rank-deficient." << endl;
		return;

	}

	map<int,typename map<size_t,T>::iterator> bands;
	typename map<int,map<size_t,T> >::iterator it_b;

	// init iterators to the beginning of all bands
	for(it_b=m_data.begin(); it_b!=m_data.end();it_b++)
		bands.insert(pair<int,typename map<size_t,T>::iterator >(it_b->first,it_b->second.begin()));

	typename map<size_t,T>::iterator it_d;
	typename map<int,typename map<size_t,T>::iterator >::iterator it_bands;

	it_d = it_diagonal->second.begin();

	// one accumulator for each column of b
	T* sums = new T[b.NCols()];

	// iterate through main diagonal
	while(it_d!=it_diagonal->second.end()) {

		for(size_t bc=0; bc<b.NCols(); bc++)
			sums[bc] = b.Get(it_d->first,bc);

		for(it_bands=bands.begin(); it_bands!=bands.end(); it_bands++) {

			// go through all band iterators and compute their row/column representation
			size_t i = CSparseBandedArray<T>::Row(it_bands->first,it_bands->second->first);
			size_t j = CSparseBandedArray<T>::Col(it_bands->first,it_bands->second->first);

			// check whether the band contributes to the current row
			if(i==it_d->first && j<it_d->first) {

				for(size_t bc=0; bc<b.NCols(); bc++)
					sums[bc] -= it_bands->second->second*x(j,bc);

				// increment the iterator in the band
				it_bands->second++;

			}

		}

		for(size_t bc=0; bc<b.NCols(); bc++) {
			x(it_d->first,bc) = sums[bc]/it_d->second;
			//cout << sums[bc]/it_d->second << endl;
		}


		it_d++;

	}

	delete [] sums;

}


template class CSparseLowerTriangularArray<float>;
template class CSparseLowerTriangularArray<double>;

}

