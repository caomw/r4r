# -*- coding: utf-8 -*-
"""
Created on Fri Dec 13 17:57:20 2013

@author: jbalzer
"""

import numpy as np
import scipy.sparse as sparse
import matplotlib.pyplot as plt
#from mpl_toolkits.mplot3d import Axes3D
import matplotlib.cm as cm
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
            print self.data
            
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
        locations given in t.     
        """

        #FIXME: This has to be a member of the spline class.
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
        return bs.find_knot_span(self.data,self.p,t)
        
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
        
        #plt.legend()
        plt.vlines(dom,0,ymax,linestyles='dashed')
        
    def number_of_controlpoints(self):
        return self.ncp
    
    def greville_abscissae(self):
        
        result = np.zeros(self.ncp)
        
        for i in range(0,self.ncp):
            result[i] = np.sum(self.data[i:i+self.p])/float(self.p)
            
        return result
                
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
            x,xt,xtt = bs.evaluate_curve(self.knots.data,self.cp,self.knots.p,t)
            xt = 0*xt
            xtt = 0*xtt
            return x,xt,xtt
            
    def evaluate_batch(self,t):
        """
        Evaluates the spline at multiple parameter locations.
        """
        x = np.zeros((self.cp.shape[0],t.size))
        xt = np.zeros((self.cp.shape[0],t.size))
        xtt = np.zeros((self.cp.shape[0],t.size))
        
        for i in range(0,t.size):
        
            temp = bs.evaluate_curve(self.knots.data,self.cp,self.knots.p,t[i])
            x[:,i] = temp[0]
            
            if self.knots.p>=1:        
                xt[:,i] = temp[1]
                xtt[:,i] = temp[2]

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
        
    def make_identity(self):
        """
        Interprets a 2d-curve as a scalar-valued functions, and sets it to the
        identity with the help of the Greville abscissae. 
        """
        
        if(self.d==1):   
            self.cp = np.reshape(self.knots.greville_abscissae(),(1,self.knots.ncp))
    
    def insert_knot(self,t):
        """
        Insert a new knot without changing the geometry of the curve.
        """        
        
        # find knot span        
        span = self.knots.get_span(t)
    
        # keep all but p of them
        cpsnew = np.zeros((self.d,self.knots.ncp+1),self.cp.dtype)
        cpsnew[:,0:span-self.knots.p+1] = self.cp[:,0:span-self.knots.p+1]
        cpsnew[:,span+1:] = self.cp[:,span:]
        
        for i in range(span-self.knots.p+1,span+1):
            denom = self.knots.data[i+self.knots.p] - self.knots.data[i]
            w0 = t - self.knots.data[i]
            w1 = self.knots.data[i+self.knots.p] - t            
            c0 = self.cp[:,i%self.knots.ncp]
            c1 = self.cp[:,(i-1)%self.knots.ncp]
            cpsnew[:,i] = (w0*c0 + w1*c1)/denom
            
        self.cp = cpsnew
        self.knots.ncp += 1
        
        # insert t in to knot vector
        self.knots.data = np.insert(self.knots.data,span+1,t)
               
# FIXME: make this a member, derive the graph and override           
def plot_curve(s,n,controlpoints=False,knotpoints=False,normals=False,annotatespan=-1):
    """
    Plots spline curve or graph.
    """

    # get domain
    domain = s.knots.get_domain();
    t = np.linspace(domain[0],domain[-1],n)
    
    # what about other cases
    if(s.d==2):
        
        if knotpoints is True:

            # mark boundaries of knot intervals
            for i in range(0,domain.size):
                x,xt,xtt = s.evaluate(domain[i])
                plt.plot(x[0],x[1],marker='x',markersize=10)
                
                if annotatespan>0 and i%annotatespan==0:
                    plt.annotate('{:.2f}'.format(domain[i]), xy=(x[0], x[1]),xytext=(x[0]+0.01, x[1]+0.01))
                
        pts = np.zeros((t.size,2))

        for i in range(0,t.size):
            x,xt,xtt = s.evaluate(t[i])
            
            if normals is True:
                plt.quiver(x[0],x[1],xt[1],-xt[0],width=0.0025)
            pts[i,:] = x
        
        plt.plot(pts[:,0],pts[:,1])
        
        
        if controlpoints is True:
            plt.plot(s.cp[0,:],s.cp[1,:],marker='o')
        
    elif(s.d==1):

        f = np.zeros(t.size)
        for i in range(0,t.size):
            f[i] = s.evaluate(t[i])[0]
            
        plt.plot(t[:-1],f[:-1])

        if controlpoints is True:
            gv = s.knots.greville_abscissae()
            plt.plot(gv,s.cp[0,:],marker='o')
    
    plt.axis('equal')
    plt.grid('on')
    #plt.show()

class surface:
 
    def __init__(self,knots,d):
        self.knots = knots
        self.d = d
        self.cp = np.zeros((d,knots[0].ncp,knots[1].ncp),dtype='double')

    def set_function(self,f):
        """
        Interprets a 3d-surface as a scalar-valued functions over the plane,
        and sets it to the identity with the help of the Greville abscissae. 
        """
        gau  = self.knots[0].greville_abscissae()
        gav  = self.knots[1].greville_abscissae()
        gu = np.outer(gau,np.ones(gav.size))
        gv = np.outer(np.ones(gau.size),gav)

        if(self.d==1):           
            self.cp[0,:,:] = f(gu,gv)            
        elif(self.d==3):
            self.cp[0,:,:] = gu
            self.cp[1,:,:] = gv
            self.cp[2,:,:] = f(gu,gv)

    def evaluate(self,t):
        """
        Evaluate the spline surface at a given parameter tuple.
        """
        x,xu,xv,xuu,xuv,xvv = bs.evaluate_curve(self.knots[0].data,self.knots[1].data,self.cp,self.knots[0].p,self.knots[1].p,t[0],t[1])
        
        if self.knots[0].p<2 or self.knots[1].p<2<2:        
            xu = 0*xu
            xv = 0*xv
            xuu = 0*xuu
            xuv = 0*xuv
            xvv = 0*xvv
      
        # do xu,xv,xuu,xuv,xvv
        return x,xu,xv,xuu,xuv,xvv
        
    def evaluate_batch(self,t):
        """
        Evaluates the spline surface at multiple parameter locations.
        """               
        x = np.zeros((self.cp.shape[0],t[0].size))
        xu = np.zeros((self.cp.shape[0],t[0].size))
        xv = np.zeros((self.cp.shape[0],t[0].size))
        xuu = np.zeros((self.cp.shape[0],t[0].size))
        xuv = np.zeros((self.cp.shape[0],t[0].size))
        xvv = np.zeros((self.cp.shape[0],t[0].size))

        if(t[0].size==t[1].size):
            
            for i in range(0,t[0].size):
            
                temp = self.evaluate((t[0][i],t[1][i]))
                x[:,i] = temp[0]
                xu[:,i] = temp[1]
                xv[:,i] = temp[2]
                xuu[:,i] = temp[3]
                xuv[:,i] = temp[4]
                xvv[:,i] = temp[5]
     
        return x,xu,xv,xuu,xuv,xvv

    def curvature(self,t):
        """
        Second fundamental form, Gauss, and mean curvature. 
        """
        x,xu,xv,xuu,xuv,xvv = self.evaluate((t[0],t[1]))
        n = np.cross(xu,xv)
        n = n/np.linalg.norm(n)
        
        II = np.zeros((2,2))
        II[0,0] = np.inner(n,xuu)
        II[0,1] = np.inner(n,xuv)
        II[1,0] = II[0,1]
        II[1,1] = np.inner(n,xvv)
        
        kappa = 0.5*(II[0,0]+II[1,1])
        gamma = II[0,0]*II[1,1] - II[0,1]+II[1,0]
        
        return kappa, gamma, II
        
    def principal_curvature_field(self,n):
        """
        Eigen-decomposition of the second fundamental form at a point to 
        find principal curvatures and their directions.
        """
        u,v = self.get_parameter_grid(n)
        maxu = np.zeros(u.shape)
        maxv = np.zeros(u.shape)
        minu = np.zeros(u.shape)
        minv = np.zeros(u.shape)
        maxvalu = np.zeros(u.shape)
        maxvalv = np.zeros(u.shape)
        
        for i in range(0,u.shape[0]):        
            for j in range(0,u.shape[1]):
                kappa, gamma, II = self.curvature((u[i,j],v[i,j]))
                vals,vecs = np.linalg.eigh(II)
                maxvalu[i,j] = vals[0]                
                maxvalv[i,j] = vals[1]
                maxu[i,j] = vecs[0,0]
                maxv[i,j] = vecs[1,0]
                minu[i,j] = vecs[0,1]
                minv[i,j] = vecs[1,1]
        
        return u, v, maxu, maxv, minu, minv, maxvalu, maxvalv
    
    def get_parameter_grid(self,n):

        if(self.d!=1):
            domains = (self.knots[0].get_domain(),self.knots[1].get_domain())
            return np.meshgrid(np.linspace(domains[0][0],domains[0][-1],n[0]),np.linspace(domains[1][0],domains[1][-1],n[1]))
        else:
            return np.meshgrid(self.knots[0].greville_abscissae(),self.knots[1].greville_abscissae())
            
        
    def plot(self,n,controlpoints=False,curvature='none'):
        """
        Plot surface, its control points, and curvature.
        """
        
        if(self.d==3 or self.d==1):
            
            u,v = self.get_parameter_grid(n) 
            print u.shape
            # substitute controlpoints
            if(self.d==1):
                self.cp = np.vstack((np.reshape(u,(1,u.shape[0],u.shape[1])),np.reshape(v,(1,v.shape[0],v.shape[1])),self.cp))

            print self.cp.shape
            

            x = np.zeros(u.shape)
            y = np.zeros(u.shape)
            z = np.zeros(u.shape)
            colors = np.zeros(u.shape)
                    
            cblabel = r'$z$'
            for i in range(0,u.shape[0]):
                for j in range(0,u.shape[1]):
                    
                    data = self.evaluate((u[i,j],v[i,j]))
                    x[i,j] = data[0][0]
                    y[i,j] = data[0][1]
                    z[i,j] = data[0][2]
                    
                    n = np.cross(data[1],data[2])
                    n = n/np.linalg.norm(n)
            
                    xuun = np.inner(n,data[3])
                    xuvn = np.inner(n,data[4])
                    xvvn = np.inner(n,data[5])
        
                    if(curvature!='none'):
                        
                        if(curvature=='mean'):
                            colors[i,j]=0.5*(xuun+xvvn)
                            cblabel = 'Mean curvature'
                        elif (curvature=='Gauss'):
                            colors[i,j]=xuun*xvvn-xuvn**2
                            cblabel = 'Gauss curvature'
                    else:
                        colors[i,j] = z[i,j]
                                
            m = cm.ScalarMappable(cmap=cm.jet)
            m.set_array(colors)
            colmax = colors.max()
            if colmax==0:
                colmax=1.0                       
            colors = colors/colmax
            
            fig = plt.figure()
            ax = fig.add_subplot(111,projection='3d')
            ax.plot_surface(x,y,z,rstride=1,cstride=1,shade=True,edgecolor='none',facecolors=cm.jet(colors),linewidth=0)
            cb=plt.colorbar(m)
            cb.set_label(cblabel, labelpad=10)
            
            
            if controlpoints is True:
                ax.plot_wireframe(self.cp[0,:,:],self.cp[1,:,:],self.cp[2,:,:],color='red')
                ax.scatter(self.cp[0,:,:],self.cp[1,:,:],self.cp[2,:,:],c='red',edgecolor='none')
                
                
            if(self.d==1):
                self.cp = np.reshape(self.cp[2,:,:],(self.cp.shape[1],self.cp.shape[2]))    
            
                    
    def assemble_interpolation_matrix(self,t):
        """
        Creates a sparse matrix needed for interpolating points given at the
        locations given in t.     
        """
        ii = []
        jj = []
        vals = []
        
        if len(t)!=2 or t[0].size!=t[1].size:
            print "Dimension mismatch..."
            return
        
        for i in range(0,t[0].size):
                        
            Nu,spanu = bs.cox_de_boor(self.knots[0].data,self.knots[0].p,t[0][i])
            Nv,spanv = bs.cox_de_boor(self.knots[1].data,self.knots[1].p,t[1][i])
            
            for k in range(0,self.knots[0].p+1):
            
                for l in range(0,self.knots[1].p+1):
            
                    # control point indices 
                    cpu = self.knots[0].convert_index(spanu-(self.knots[0].p-1),k)
                    cpv = self.knots[1].convert_index(spanv-(self.knots[1].p-1),l)

                    # order of the abscissae (rows) is arbitrary          
                    ii.append(i)
                    
                     # make sure to use the same ordering to set a result             
                    jj.append(cpu*self.cp.shape[2]+cpv)
                    
                    vals.append(Nu[0,k]*Nv[0,l])

        # convert to numpy arrays
        ii = np.array(ii,dtype='int')
        jj = np.array(jj,dtype='int')
        vals = np.array(vals,dtype='double')
        
        A = sparse.coo.coo_matrix((vals,np.vstack((ii,jj))))
        A = A.tocsc()
        
        return A


    def interpolate(self,t,pts):
        """
        Initialize control points by interpolation.
        """
        if(len(pts)!=self.d):
            print "Dimension mismatch..."
            return
        
        A = self.assemble_interpolation_matrix(t) 
        
        for i in range(0,self.d):
            result = lsqr(A,pts[i])
            self.cp[i,:,:] = np.reshape(result[0],(self.cp.shape[1],self.cp.shape[2]))                
                
