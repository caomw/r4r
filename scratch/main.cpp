#include <iostream>
#include <fstream>
#include "darray.h"
#include "descriptor.h"
#include "feature.h"
#include "basic.h"
#include "rect.h"
#include "tracklet.h"
#include "params.h"
#include "descspecial.h"

using namespace std;
using namespace R4R;
using namespace cv;

int main()
{


   std::vector<CDescriptorFileHeader> headers;

   ETYPE type;
   cout << type << endl;

   float* data = (float*)CFeature::LoadDescriptors("/home/jbalzer/ficken.txt",headers,type);


   cout << type << endl;
   matf img(61,61,data+500*61*61);

   cout << img << endl;

   //ofstream mout("img.txt");
   //mout << img << endl;
   //mout.close();


//    CRectangle<double> roi = CRectangle<double>(0,0,1,1,0);
//    mat B(3,3);
//    B.Rand();
//    CDescriptor<mat>* desc = new CDescriptor<mat>(B);

//    cout << B << endl;

//    CFeature x(0,0,0,0);


//    shared_ptr<CDescriptor<mat> > ptr = shared_ptr<CDescriptor<mat> >(desc);
//    x.AttachDescriptor("ID",ptr);
//    x.AttachDescriptor("ID2",ptr);


//    list<CFeature> out, in;
//    out.push_back(x);
//    //out.push_back(x);

//    CFeature::SaveToFile("newtest.txt",out);

//    CFeature::OpenFromFile("newtest.txt",in);

//    list<CFeature>::iterator it;

//    for(it=in.begin(); it!=in.end(); it++) {

//        cout << *it << endl;

//        if(it->HasDescriptor("ID")) {

//            shared_ptr<CAbstractDescriptor> pdesc = it->GetDescriptor("ID");

//            cout << pdesc->GetType() << endl;
//            CDescriptor<mat>* desc = (CDescriptor<mat>*)pdesc.get();

//            mat& container = desc->Get();
//            cout << container << endl;


//        }

//        if(it->HasDescriptor("ID2")) {

//            shared_ptr<CAbstractDescriptor> pdesc = it->GetDescriptor("ID2");

//            cout << pdesc->GetType() << endl;
//            CDescriptor<mat>* desc = (CDescriptor<mat>*)pdesc.get();

//            mat& container = desc->Get();
//            cout << container << endl;


//        }


//    }




}

