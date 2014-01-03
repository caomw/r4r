# -*- coding: utf-8 -*-
"""
Created on Fri Dec 13 17:57:20 2013

@author: jbalzer
"""

import numpy as np
import scipy.sparse as sparse
import matplotlib.pyplot as plt
from scipy.sparse.linalg import lsqr

import r4r.core._bsplines as bs

def unwrap_phase(xw):
    """
    Select correct branch of arctan2 function (phase-unwrapping).
    """
    return bs.unwrap_phase(xw)

class knot_vector:
    
    def __init__(self,p,periodic):
        self.p = p
        self.periodic = periodic
        
    def make_uniform(self,l,u,n):
        """
        Create a uniformly-spaced open or periodic knot vector.
        """
        if self.p==1:
            self.data = np.linspace(l,u,n)
            
            if self.periodic:
                self.ncp = self.data.size-1
            else:
                self.ncp = self.data.size
                            
            return
            
        if self.periodic:
                
            # compute boundary ghost knots    
            middle = np.linspace(l,u,n)
            lower = middle[0] + np.cumsum(middle[-self.p:-1]-middle[(-self.p+1):])    
            upper = middle[-1] + np.cumsum(- middle[0:self.p-1] + middle[1:self.p])
            knots = np.concatenate((lower[::-1],middle,upper))
            self.data = knots
            
            # set number of basis functions            
            self.ncp = self.data.size - 2*self.p + 1
            
        else:
            
            # compute ghost knots
            lower = np.repeat(np.array([l]),self.p-1)
            upper = np.repeat(np.array([u]),self.p-1)
            middle = np.linspace(l,u,n)
            self.data = np.concatenate((lower,middle,upper))
            
            # number of basis functions
            self.ncp = self.data.size - self.p + 1
            
    def assemble_interpolation_matrix(self,t):
        """
        Creates a sparse matrix needed for interpolating points given at the
        locations given in t    
        """
        ii = []
        jj = []
        vals = []
        
        for i in range(0,t.size):
            
            N,span = bs.cox_de_boor(self.data,self.p,t[i])
            
            for j in range(0,self.p+1):
                
                index = self.convert_index(span-(self.p-1),j)
                ii.append(i)
                jj.append(index)
                vals.append(N[0,j])

        # convert to numpy arrays
        ii = np.array(ii,dtype='int')
        jj = np.array(jj,dtype='int')
        vals = np.array(vals,dtype='double')
        
        A = sparse.coo.coo_matrix((vals,np.vstack((ii,jj))))
        A = A.tocsc()
        
        return A
    
    def get_domain(self):
        """
        Removes the ghost knots from knot vector.
        """
        if self.p==1:
            return self.data
        else:
            return self.data[self.p-1:-self.p+1]
    
    def get_span(self,t):
        return bs.knot_span(self.data,self.p,t)
        
    def convert_index(self,span,i):
        """         
        Converts knot span plus local basis function index to the global
        index of the basis function. 
        """
        
        # need mod operation for periodic spline, has no effect otherwise
        return (span + i)%self.ncp
    

    def compute_basis_function(self,t,i):
        """
        Return the value of a single basis function at a given parameter.
        """
        
        N,span = bs.cox_de_boor(self.data,self.p,t)
            
        for j in range(0,self.p+1):
               
            index = self.convert_index(span-(self.p-1),j)
            
            if(index==i):
                return N[0,j]
                
        return 0
            
    def compute_basis_functions(self,t):
        """
        Computes all basis functions over a given range of parameters and
        stores them on top of each other in an array.
        """
        result = np.zeros((self.ncp,t.shape[0]))
        
        for i in range(0,t.shape[0]):
      
            N,span = bs.cox_de_boor(self.data,self.p,t[i])

            for j in range(0,self.p+1):
                
                index = self.convert_index(span-(self.p-1),j)
                result[index,i] = N[0,j]
   
        return result

    def plot_basis(self,n):
        """
        Plots the scalar basis functions defined by the knot vector.
        """
        dom = self.get_domain()    
        t = np.linspace(dom[0],dom[-1],n)
        basis = self.compute_basis_functions(t)
        ymax = np.max(basis)
        
        for i in range(0,basis.shape[0]):    
            plt.plot(t,basis[i,:],label=str(i))
        
        plt.legend()
        plt.vlines(dom,0,ymax,linestyles='dashed')
        
class curve:
 
    def __init__(self,knots,d):
        self.knots = knots
        self.d = d
        self.cp = np.zeros((d,knots.ncp),dtype='double')

    def interpolate(self,t,pts):
        """
        Initialize control points by interpolation.
        """
        
        A = self.knots.assemble_interpolation_matrix(t) 
        
        for i in range(0,self.d):

            result = lsqr(A,pts[i])
            self.cp[i,:] = result[0]
        
    def evaluate(self,t):
        """
        Evaluate the spline at a given parameter location.
        """
        
        if self.knots.p>=2:        
            return bs.evaluate_curve(self.knots.data,self.cp,self.knots.p,t)
        else:
            print "ERROR: Degree must be greater than 2.\n"
            x = np.zeros(self.cp.shape[0])
            xt = np.zeros(self.cp.shape[0])
            xtt = np.zeros(self.cp.shape[0])
            return x,xt,xtt
            
    def distance(self,x0,n):
        """
        Sample the distance of a point to the curve.
        """
        
        # get domain
        domain = self.knots.get_domain();
        t = np.linspace(domain[0],domain[-1],n)

        result = []        
        for i in range(0,t.size):
            x,xt,xtt = self.evaluate(t[i])
            result.append(np.linalg.norm(x-x0))
        return np.array(result)
    
    def sample_location(self,n):
        """
        Sample curve locations at given density.
        """

        domain = self.knots.get_domain();
        t = np.linspace(domain[0],domain[-1],n)
        
        result = []        
        for i in range(0,t.size):
            x,xt,xtt = self.evaluate(t[i])
            result.append(x)
        return np.array(result)
        
    def hodograph_phase(self,n):
        """
        Phase of the normalized tangent vector field. 
        """
        # this works only in 2d
        if self.d != 2:
            print "Warning: Curve is not planar."
            return np.zeros(n)
            
        # get domain
        domain = self.knots.get_domain();
        t = np.linspace(domain[0],domain[-1],n)

        result = []        
        for i in range(0,t.size):
            x,xt,xtt = self.evaluate(t[i])
            phi = np.arctan2(xt[1],xt[0])
            result.append(phi)
            
        return np.array(result)
        
    def closest_point(self,x0,t0,eps):
        """
        Find the closest point on the curve by Newton's method.
        
        TODO: Sample different starting values depending on spectral characteristics
        of unwrapped hodograph phase. 
        """        
        
        tk = t0
        xk,xtk,xttk = self.evaluate(tk)
        r = x0 - xk            
        rno = np.linalg.norm(r)
        
        rs = []
        rs.append(rno)
        
        for i in range(0,10):
    
            num = np.inner(r,xtk)
            denom = np.inner(r,xttk) - np.inner(xtk,xtk)             
            tk = tk - num/denom 

            xk,xtk,xttk = self.evaluate(tk)
            r = x0 - xk
      
            rn = np.linalg.norm(r)
              
            if abs(rn-rno)<eps:
                break
            else:
                rs.append(rn)
                rno = rn
     
        return xk,tk,rs
        
    def bounding_box(self):
        """
        Compute bounding box of curve using the convex hull property of B-spline
        curves.
        """
        mins = np.min(self.cp,1)
        maxs = np.max(self.cp,1)
        return np.vstack((mins,maxs)).transpose()
        
    def characteristic_function(self,x,y,n):
        """
        Characteristic function of domain bounded by the curve. 
        """
        
        result = np.zeros(x.shape)

        if self.d != 2:
            print "Warning: Curve is not planar."
            return result

        # sample the curve
        cs = self.sample_location(n)
        return bs.characteristic_function(x,y,cs)
        
                 
# fixme: make this a member, derive the graph and override           
def plot_curve(s,n,controlpoints=False,knotpoints=False,normals=False):
    """
    Plots spline curve or graph.
    """

    # get domain
    domain = s.knots.get_domain();
    t = np.linspace(domain[0],domain[-1],n)
    
    if(s.d==2):
        
        if knotpoints is True:

            # mark boundaries of knot intervals
            for i in range(0,domain.size):
                x,xt,xtt = s.evaluate(domain[i])
                plt.plot(x[0],x[1],marker='x',color='g',markersize=10)
                #plt.annotate(str(i), xy=(xk[0], xk[1]),xytext=(xk[0]+0.01, xk[1]))
                
        pts = np.zeros((t.size,2))

        for i in range(0,t.size):
            x,xt,xtt = s.evaluate(t[i])
            
            if normals is True:
                plt.quiver(x[0],x[1],xt[1],-xt[0],width=0.0025)
            pts[i,:] = x
        
        plt.plot(pts[:,0],pts[:,1])
        
        
        if controlpoints is True:
            plt.plot(s.cp[0,:],s.cp[1,:],marker='o',color='r')
        
    elif(s.d==1):

        f = np.zeros(t.size)
        for i in range(0,t.size):
            f[i] = s.evaluate(t[i])
            
        plt.plot(t[:-1],f[:-1])

        gv = s.knots.greville_abscissae()
        plt.plot(gv,s.cp[0,:],marker='o',color='r')
    
    plt.axis('equal')
    plt.grid('on')
    #plt.show()