{
    gROOT->Reset();
    gStyle->SetPalette(1);
    
    double L=11.9;     
    double Fconv= 0.25; //TDC: ns per canale...
    
    double t0[5]={0, -34.5 ,-23.89, -16.66, -8.99};  //setup 3mattine 2016

    double adc_peds[5]={0.,0.,0.,0.,0.};
    double adc_mips[5]={1.,1.,1.,1.,1.};
    
    string file_adc="ADC.dat";
    string file_tdc="TDC.dat";

   

    int Nev, time, tdc1, tdc2, tdc3, tdc4, tdc5, tdc6;
    int Nev_adc,  adc1, adc2, adc3, adc4, adc5, adc6;

    double x_scala[2]={-2,14};
    double y_scala[2]={-2,14};
    
    double x[5]={L/2., L,L,0,0};
    double y[5]={L/2.,L,0,0,L};

    TCanvas *c1 = new TCanvas("c1","",0);
   
    TGraph *g_array=new TGraph(5,x,y);
    g_array->SetMarkerStyle(20);
   
    TGraph *g_scala=new TGraph(2,x_scala,y_scala); 
    g_scala->GetXaxis().SetTitle("x [m]");
    g_scala->GetYaxis().SetTitle("y [m]");
    
    int prev_event=-1;   
  while(1){ // loop infinito letture files...
    //////////////////////////////////////////////////////////////////////
    ifstream in1 (file_tdc.c_str());
    ifstream in2 (file_adc.c_str());
    
    while (1) {
        in1 >> Nev >> time >> tdc1 >> tdc2 >> tdc3 >> tdc4 >> tdc5 >> tdc6;
	in2 >> Nev_adc >> adc1 >> adc2 >> adc3 >> adc4 >> adc5 >> adc6;
	
	if (in1.eof() || in2.eof() ) break;
   
    } // end loop file... 

    cout<<"ultimo Evento:   Nev="<<Nev<<" nev zdc = "<<Nev_adc<<" prev EV = "<<prev_event<<endl;
   
    if (Nev==prev_event){
       gSystem->Sleep(5000);
       c1->Update();
       continue;    
    }
	
   c1->Clear();
   char title[100];
   sprintf(title,"event n. %d",Nev);
   g_scala->SetTitle(title);
   double tdc[5]={tdc1,tdc2,tdc3,tdc4,tdc5};
   double dT[5];
		
   double size[5];
   size[0]=adc1;
   size[1]=adc2;
   size[2]=adc3;
   size[3]=adc4;
   size[4]=adc5;

   double xb=0;
   double yb=0;
   double sumSize=0;

   g_scala->Draw("ap");
   g_array->Draw("p");

   TMarker *s[5];      
   for (int i=0; i<5; i++){ 
	  
	  dT[i]= (tdc[i] - tdc[0] + t0[i])*Fconv;
	  int color =0;//   int(51 + (dT[i] )/(maxT - minT)*49.);
	  if (dT[i]<=-10) {color=4;}
	  else if (dT[i]<=0 && dT[i]>-10) {color=3;}
	  else if (dT[i]>0 &&dT[i]<10){color=5;}
	  else if (dT[i]>=10) {color=2;}
	  //  cout<<" i = "<<i<<" dT = "<<dT[i]<< "   color = "<<color<<endl;
	 
	  size[i]=(size[i]-adc_peds[i])/adc_mips[i];  //ADC calibration!!!!!!

	  xb=xb+size[i]*x[i];
	  yb=yb+size[i]*y[i];
	  sumSize=sumSize+size[i];
	  
	  size[i]=size[i]/100.;
	  if (size[i]<0.5)size[i]=0.5;
	  s[i]=new TMarker(x[i],y[i],20); 
	  s[i]->SetMarkerColor(color); 
	  s[i]->SetMarkerSize(size[i]);
	  s[i]->Draw();
   }
   if (sumSize!=0){
        xb=xb/sumSize;
	yb=yb/sumSize;
 	cout<<"xb = "<<xb<<"yb= "<<yb<<endl;
	TMarker *bary=new TMarker(xb,yb,25);
	bary->Draw();
   }
   c1->Update();

   in1.close();
   in2.close();
   ////////////////////////////////////////////////////////     


   prev_event=Nev;
  }// end loop letture



   //  c1->Clear();
   //delete s;
   //}//end loop lettura file	
	    

   


  }
