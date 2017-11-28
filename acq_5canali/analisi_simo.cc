//
//  analisi.cc
//  
//
//  Created by Riccardo Aliberti on 24/02/12.
//

#include "Riostream.h"
#include "TH1D.h"
#include "TMath.h"
#include "TTree.h"
#include "TFile.h"
#include "TF1.h"
#include "TStyle.h"
#include "TROOT.h"

#define velc 0.3 //in m/ns
#define L 11.9
#define NBins 100

//string file_adc="./Esperienza_EAS/2012/adc_all.dat";
//string file_tdc="./Esperienza_EAS/2012/tdc_all.dat";

string file_adc="ADC_1.dat";
string file_tdc="TDC_1.dat";



double t0[5]={0, -15.14 ,-1.97,-3.97, 2.7}; // offeset di ogni canale





string outfile_name="ricostruzione_lunga.root";


double Fconv = 0.16;

gStyle -> SetOptFit(1);

void analisi_simo() {

    ofstream scrivi ("dir_eventi.dat"); 

    
    TFile out (outfile_name.c_str(),"recreate");
    
    //importa da file tdc
    ifstream in1 (file_tdc.c_str());
    ifstream in2 (file_adc.c_str());
    
    TH1D *C1 = new TH1D ("costante_2-1","costante_2-1",1200,-300,300);
    TH1D *C2 = new TH1D ("costante_3-1","costante_3-1",1200,-300,300);
    TH1D *C3 = new TH1D ("costante_4-1","costante_4-1",1200,-300,300);
    TH1D *C4 = new TH1D ("costante_5-1","costante_5-1",1200,-300,300);
    
    
    // istogrammi spettri adc
    
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
    
    
    
    int Nev, time, tdc1, tdc2, tdc3, tdc4, tdc5, tdc6;
    int Nev_adc,  adc1, adc2, adc3, adc4, adc5, adc6;
    
    vector <double> tdc[5];
    while (1) {
        
	in1 >> Nev >> time >> tdc1 >> tdc2 >> tdc3 >> tdc4 >> tdc5 >> tdc6;
	in2 >> Nev_adc >> adc1 >> adc2 >> adc3 >> adc4 >> adc5 >> adc6;
	
	if (in1.eof() || in2.eof() ) break;
	
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
            tdc[0].push_back(tdc1*Fconv);
            tdc[1].push_back(tdc2*Fconv);
            tdc[2].push_back(tdc3*Fconv);
            tdc[3].push_back(tdc4*Fconv);
            tdc[4].push_back(tdc5*Fconv);
            
            C1 -> Fill((tdc2 - tdc1)*Fconv);
            C2 -> Fill((tdc3 - tdc1)*Fconv);
            C3 -> Fill((tdc4 - tdc1)*Fconv);
            C4 -> Fill((tdc5 - tdc1)*Fconv);
        
	    
	      
	
	}
    }
    
        
    int N = tdc[0].size();
    double Coffset[5];
    
    TF1* Cgauss1 = new TF1("C1gaus","gaus");
    TF1* Cgauss2 = new TF1("C2gaus","gaus");
    TF1* Cgauss3 = new TF1("C3gaus","gaus");
    TF1* Cgauss4 = new TF1("C4gaus","gaus");
    
    C1->Fit("C1gaus","F");
    C2->Fit("C2gaus","F");
    C3->Fit("C3gaus","F");
    C4->Fit("C4gaus","F");    
    
    Coffset[0] = 0;
    Coffset[1] = Cgauss1 -> GetParameter(1);
    Coffset[2] = Cgauss2 -> GetParameter(1);
    Coffset[3] = Cgauss3 -> GetParameter(1);
    Coffset[4] = Cgauss4 -> GetParameter(1);
    
    
    
    TH1D *distTheta = new TH1D ("dist_theta","#theta",NBins,0,TMath::PiOver2());
    TH1D *distPhi = new TH1D ("dist_phi","#phi",NBins,0,2.*TMath::Pi());

    TH1D *distThetaDeg = new TH1D ("dist_thetaDeg","#theta",900,0,90);
    TH1D *distPhiDeg = new TH1D ("dist_phiDeg","#phi",3600,0,360);


    
    TH1D *distDT = new TH1D ("dist_DT","#sigma_{T_{tot}}",200,-50,50);
    TH1D *distDX = new TH1D ("Spessore sciame","Spessore Sciame",200,-50,50);
    
    
    struct dir {
        double x,y,z,sp_ch;
    };
    
    dir Direzione;
    
    TTree *tree = new TTree ("Direzioni","Dir");
    tree -> Branch("direzioni",&Direzione.x,"x/D:y:z:sp_ch");
    
    for (int i=0; i<N; i++) {
        double A,B,C;
        double T[5];
        
	for (int j=0; j<5; j++){ 
	// T[j] = tdc[j][i] - tdc[0][i] - Coffset[j];
         T[j] = tdc[j][i] - tdc[0][i] + (t0[j]*Fconv);
        
	
	}
	
	A= velc/(2*L) * ((T[1] - T[3]) - (T[4] -T[2]));
        B= velc/(2*L) * ((T[1] - T[3]) + (T[4] -T[2]));
        C= TMath::Sqrt(1 - A*A - B*B);
        
        double Theta = TMath::ACos(C);
        double Phi = TMath::ATan2(B,A);
        if (Phi < 0) Phi += 2*TMath::Pi();
        
        
        double DT = (T[4] - T[1] - T[3] + T[2]);
        
        distPhi -> Fill(Phi);
        distTheta -> Fill(Theta);
	
	distPhiDeg -> Fill(Phi*TMath::RadToDeg());
        distThetaDeg -> Fill(Theta*TMath::RadToDeg());
	
	scrivi<<"theta = "<<Theta*TMath::RadToDeg()<<" phi = "<< Phi*TMath::RadToDeg()<<endl;
	
        distDT -> Fill(DT);
        distDX -> Fill(DT*velc);
        
        Direzione.x=A;
        Direzione.y=B;
        Direzione.z=C;
        Direzione.sp_ch= (velc * (T[4] - T[1] - T[3] + T[2]) /2.);
        
        tree -> Fill();
    }
    
    
    //normalizzazione h tete e phi...
   // for (int i=1; i<=NBins; i++) {
   //     distPhi -> SetBinContent(i,distPhi->GetBinContent(i)/N);
   //     distTheta -> SetBinContent(i,distTheta->GetBinContent(i)/N);
   // }
    
    TF1 *retta_phi = new TF1 ("retta_phi","pol1",0,2.*TMath::Pi());
    TF1 *fTheta = new TF1 ("fTheta","[0]*(sin(x)*(cos(x)^[1]))",0,0.6);
    fTheta->SetParameter(1,2);
    distTheta -> Fit("fTheta","R");
   
    TF1 *fDT = new TF1 ("fDT","gaus");
    //fDT->SetParameter(1,4.);
    TF1 *fDX = new TF1 ("fDX","gaus");
    //fDX -> SetParameter(1,4.*velc);
    
    distPhi -> Fit("retta_phi");
    //distTheta -> Fit("fTheta");
    distDT -> Fit("fDT");
    distDX -> Fit("fDX");
    
    distPhi -> GetXaxis() -> SetTitle("#phi (diviso in 100 bin)");
    //distPhi -> GetYaxis() -> SetTitle("frequenza eventi (normalizzata a 1)");
    distPhi -> GetYaxis() -> SetTitle("n. eventi ");
    
    distTheta -> GetXaxis() -> SetTitle("#theta (diviso in 100 bin)");
    //distTheta -> GetYaxis() -> SetTitle("frequenza eventi (normalizzata a 1)");
    distTheta -> GetYaxis() -> SetTitle("n. eventi");
    
    //
    distDX -> GetXaxis() -> SetTitle("Spessore (m)");
    distDX -> GetYaxis() -> SetTitle("Neventi");
    distDT -> GetXaxis() -> SetTitle("#sigma_{T_{tot}} (ns, in 100 bins)");
    distDT -> GetYaxis() -> SetTitle("Neventi");
    
    
    //distPhi -> SetStats(kFALSE);
    //distTheta -> SetStats(kFALSE);
    //distDT -> SetStats(kFALSE);
    distDX -> SetStats(kFALSE);
 
    /*distPhi -> SetFitStats(1);
    distTheta -> SetFitStats(1);
    distDT -> SetFitStats(1);
    distDX -> SetFitStats(1);*/
    
    
    distPhi -> Write();
    distTheta -> Write();
    distDT -> Write();
    distDX -> Write();

    distPhiDeg -> Write();
    distThetaDeg -> Write();


    
    C1 -> Write();
    C2 -> Write();
    C3 -> Write();
    C4 -> Write();
    
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


    
    tree -> Write();
    
    out.Close();    
}




