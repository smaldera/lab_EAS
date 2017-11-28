//
//   Semplice macro root  per leggere i files ADC.dat e TDC.dat 
//   e creare alcuni istogrammi con root   
//
// usage:    
//  >root
// root[0] .x analisi_template2.C
//


{

  gROOT->Reset();

  string file_adc="ADC_quintupla_100_eventi.dat";
  string file_tdc="TDC_quintupla_100_eventi.dat";


  double Fconv = 0.25;  // conversione canle TDC -> ns  ????????????????????

   
  //importa da file tdc
  ifstream in1 (file_tdc.c_str());
  ifstream in2 (file_adc.c_str());
  
  // crea file root output
  TFile *out=new TFile("out_file.root","recreate");

  // crea   istogrammi spettri adc
    
  TH1D *h_adc1 = new TH1D ("h_adc1","adc1",2500,-300,2200);
  TH1D *h_adc2 = new TH1D ("h_adc2","adc2",2500,-300,2200);
  TH1D *h_adc3 = new TH1D ("h_adc3","adc3",2500,-300,2200);
  TH1D *h_adc4 = new TH1D ("h_adc4","adc4",2500,-300,2200);
  TH1D *h_adc5 = new TH1D ("h_adc5","adc5",2500,-300,2200);
  TH1D *h_adc6 = new TH1D ("h_adc6","adc6",2500,-300,2200);
        
  TH1D *h_tdc1 = new TH1D ("h_tdc1","tdc1",5000,0,5000);
  TH1D *h_tdc2 = new TH1D ("h_tdc2","tdc2",5000,0,5000);
  TH1D *h_tdc3 = new TH1D ("h_tdc3","tdc3",5000,0,5000);
  TH1D *h_tdc4 = new TH1D ("h_tdc4","tdc4",5000,0,5000);
  TH1D *h_tdc5 = new TH1D ("h_tdc5","tdc5",5000,0,5000);
  TH1D *h_tdc6 = new TH1D ("h_tdc6","tdc6",5000,0,5000);

  TH1D *C1ch = new TH1D ("costante_2-1ch","costante_2-1 [ch]",600,-300,300); 
    
 
  int Nev, time, tdc1, tdc2, tdc3, tdc4, tdc5, tdc6;
  int Nev_adc,  adc1, adc2, adc3, adc4, adc5, adc6;
    
  vector <double> tdc[5];
  // loop lettura files
  while (1) {
        
	in1 >> Nev >> time >> tdc1 >> tdc2 >> tdc3 >> tdc4 >> tdc5 >> tdc6;
	in2 >> Nev_adc >> adc1 >> adc2 >> adc3 >> adc4 >> adc5 >> adc6;
	
	if (in1.eof() || in2.eof() ) break;
	   // riempie istogrammi
	    h_adc1->Fill(adc1);
	    h_adc2->Fill(adc2);
	    h_adc3->Fill(adc3);
	    h_adc4->Fill(adc4);
	    h_adc5->Fill(adc5);
	    h_adc6->Fill(adc6);
		
            h_tdc1->Fill(tdc1);
	    h_tdc2->Fill(tdc2);
	    h_tdc3->Fill(tdc3);
	    h_tdc4->Fill(tdc4);
	    h_tdc5->Fill(tdc5);
	    h_tdc6->Fill(tdc6);
	
	if (tdc1 < 2000 && tdc2 < 2000 && tdc3 < 2000 && tdc4 < 2000 && tdc5 < 2000
	     && adc1>2 && adc2>2 && adc3>2 && adc4>2 && adc5>2  ){ // cuts!!!!! 
	   
	  // riempie dei vettri con i valori TDC 
            tdc[0].push_back(tdc1*Fconv);
            tdc[1].push_back(tdc2*Fconv);
            tdc[2].push_back(tdc3*Fconv);
            tdc[3].push_back(tdc4*Fconv);
            tdc[4].push_back(tdc5*Fconv);
              
            C1ch -> Fill((tdc2 - tdc1));
  	
	} // end if cuts
  } // end loop lettura files

    
    //loop sui tempi salvati sui vettori tdc[i]
  for (int i=0; i<tdc[0].size(); i++ ){

    cout<<"i = "<<i<<" tdc[0][i]= "<<tdc[0][i]<<"  tdc[1][i]= "<<tdc[1][i]<< endl;
    
  }


    
  //esempio Fit gaussiano      
  TF1* Cgauss1ch = new TF1("C1gausch","gaus",-200,200);
  C1ch->Fit("C1gausch","MER");
 

  // salvo gli istogrammi nel file root di  output
  C1ch -> Write();
  h_adc1->Write();
  h_adc2->Write();  
  h_adc3->Write();
  h_adc4->Write();
  h_adc5->Write();
  h_adc6->Write();     

  h_tdc1->Write();
  h_tdc2->Write();  
  h_tdc3->Write();
  h_tdc4->Write();
  h_tdc5->Write();
  h_tdc6->Write();
    

  // close out root file  
  out->Close();    
}




