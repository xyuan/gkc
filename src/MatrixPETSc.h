/*
 * =====================================================================================
 *
 *       Filename: MatrixPETSc.cpp
 *
 *    Description: Matrix implementation for PETSc. Works as an abstract layer for
 *                 PETSc matrix class.
 *                 Overloading operators not complete, neither consistent. 
 *                 Improve !
 *
 *         Author: Paul P. Hilscher (2012), 
 *
 *        License: GPLv3+
 * =====================================================================================
 */

#ifndef __MatrixPETSC_H
#define __MatrixPETSC_H

#include <petscksp.h>

#include "Parallel.h"

/**
 *
 *   Class Defined Matrix implementation using PETSc (http://www.mcs.anl.gov/petsc/)
 *
  *  From the PETSc documenation (do we take care of this ? ) : 
  *     
  *     Currently, all PETSc parallel matrix formats are partitioned by
  *     contiguous chunks of rows across the processors.  Determine which
  *     rows of the matrix are locally owned.
 *   
 *
 *
 */
class Matrix
{

  Mat PETSc_Matrix;
  Parallel *parallel;
 
  int n_local;
  int n_global;

  public:
  Matrix(int _n_local, int _n_global, int dir, Parallel *_parallel) : parallel(_parallel) , n_local(_n_local), n_global(_n_global)
  {
          // Investigate which Matrix format is the fastest (Dense is not working due to PlaPACK)
          MatCreate(parallel->Comm[dir], &PETSc_Matrix);
          MatSetSizes( PETSc_Matrix, n_local, n_local, n_global, n_global);
//MatSetType ( PETSc_Matrix, MATAIJ);
          MatSetType ( PETSc_Matrix, MATAIJ);
          MatSetUp   ( PETSc_Matrix);

//          int start, stop;
//          MatGetOwnershipRange(PETSc_Matrix,&start,&stop);
  };

 /**
  *
  *  Constructor using previously instantiated PETSc matrix.
  *
  *
  */ 
  Matrix(const Mat &A) {

    PETSc_Matrix = A;
//    PetscObjectReference((PetscObject) PETSc_Matrix);
    PetscObjectReference((PetscObject) A);
    checkAndAssemble();

    };

   ~Matrix() {

       MatDestroy(&PETSc_Matrix);

   }
   
   
   /**
    *    Assemble matrix if not done (all caches are written out). 
    *
    *
    * */   
   void checkAndAssemble() 
  {
    PetscBool isAssembled;
    MatAssembled(PETSc_Matrix, &isAssembled);
    if(isAssembled == PETSC_FALSE) assemble();
    return;
  };

   void setValue(int row, int col, cmplxd value) {
              
        MatSetValues(PETSc_Matrix, 1, &row, 1, &col, &value, INSERT_VALUES);

   };


  void assemble() {

          MatAssemblyBegin(PETSc_Matrix, MAT_FINAL_ASSEMBLY);
          MatAssemblyEnd  (PETSc_Matrix, MAT_FINAL_ASSEMBLY);
  }

  Mat& getMat() {

    checkAndAssemble(); 
    
    return PETSc_Matrix;
  
  };

  //
  /**
   *   Set all Matrix entries to zero
   *
   *
   *
   *
   */ 
  void setZero() {
      MatZeroEntries(PETSc_Matrix);
  }

  
  
  
  //// Operator overloading /////////////
//  friend Matrix& operator+=(Matrix &A, cmplxd a) {
//    MatShift(A.getMat(), a);
//    return A;
//  };
  
  friend Matrix& operator+=(Matrix &A, Matrix &B) {
    MatAXPY(A.getMat(), 1., B.getMat(), DIFFERENT_NONZERO_PATTERN);
    return A;
  };
              

  /**
   *
   *   Matrix-Matrix Multiplication 
   *
   *   Note : A new Matrix is created
   *
   *
   *
   *
   * */
  friend Matrix& operator*(Matrix &A, Matrix &B) {

      // This is broken
//     std::cout << " Matrix x Matrix " << std::endl;

      Mat C;
      MatMatMult(A.getMat(), B.getMat(), MAT_INITIAL_MATRIX, PETSC_DEFAULT, &C);
 //     MatView(C,    PETSC_VIEWER_DRAW_WORLD);
      //MatMatMult(A.getMat(), B.getMat(), MAT_INITIAL_MATRIX, PETSC_DEFAULT, &C);
//      Matrix Matrix_C(C);
//      Matrix_C.checkAndAssemble();
//      wtf is this (but it works ... ) pretty sure this is a huge memory leak ... :(
//      How to do it properly ???  
      return *(new Matrix(C));

  }

//  friend Matrix& operator+(Matrix &A, cmplxd a) {
//    A.checkAndAssemble(); 
//    MatShift(A.getMat(), a);
//    return A;
//  };
  
  friend Matrix& operator*(cmplxd a, Matrix &A) {
    
      A.checkAndAssemble();
      MatScale(A.getMat(), a);
      return A;

  };
  friend Matrix& operator*(double &a, Matrix &A) {
    
      MatScale(A.getMat(), cmplxd(a,0));
      return A;
  };
  friend Matrix& operator*(Matrix &A, double &a) {
    
      A.checkAndAssemble();
      MatScale(A.getMat(), cmplxd(a,0));
      return A;
  };

  friend Matrix& operator*(Matrix &A, cmplxd a) {
      A.checkAndAssemble();
      MatScale(A.getMat(), a);
      return A;
  }

  /* 
  inline cmplxd& Matrix::operator() (int row, int col)
  {
    cmplxd value; 
    MatSetValues(PETSc_Matrix, 1, &row, 1, &col, &value, INSERT_VALUES);
    return value;
 
  };
   */
  void addDiagonal(cmplxd a) {
    checkAndAssemble(); 
    MatShift(PETSc_Matrix, a);
  }

  void addDiagonal(double a) {
    checkAndAssemble(); 
    MatShift(PETSc_Matrix, cmplxd(a, 0.));
  }

 
  /**
   *
   *   Matrix reduction over $\mu$. Shape is equal thus we don't have to worry about size.
   *
   *
   *
   *
   **/
  void reduce(int dir)
  {
          int start, stop;
//          MatGetOwnershipRange(PETSc_Matrix,&start,&stop);
//          std::cout << "Ownership range PETc_A ----> : " << start << "  / " << stop << std::endl;
          checkAndAssemble();

               Mat Matrix_AllReduce;
               
               Array2z ReduceArray(Range(0, n_local-1),  Range(0, n_local-1)); 
   //            Array2z ReduceArray(Range(start, stop),  Range(start, stop)); 
               
               MatConvert(PETSc_Matrix, MATMPIDENSE,  MAT_INITIAL_MATRIX, &Matrix_AllReduce);

               cmplxd *matrix_val;
               MatGetArray(Matrix_AllReduce, &matrix_val);

               for(int x = 0, n = 0; x < n_local-1; x++) {  for(int x2 = 0; x2 < n_local-1; x2++) {

                        ReduceArray(x,x2) = matrix_val[n++];
               } }

               // Reduction over dimension, take care that size is the same
               parallel->collect(ReduceArray, OP_SUM, dir);
               
               for(int x = 0, n = 0; x < n_local-1; x++) {  for(int x2 = 0; x2 < n_local-1; x2++) {
                    matrix_val[n++] = ReduceArray(x,x2);
               } }     
               
               MatRestoreArray(Matrix_AllReduce, &matrix_val);
               MatCopy(Matrix_AllReduce, PETSc_Matrix, DIFFERENT_NONZERO_PATTERN);

               checkAndAssemble();

               //MatConvert(Matrix_AllReduce, MATAIJ,  MAT_INITIAL_MATRIX, &PETSc_Matrix);

  };

};


#endif // __MatrixPETSC_H
