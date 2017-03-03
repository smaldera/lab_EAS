/*Librerie di C++*/
#include <stdio.h>
#include <iostream.h>
#include <fstream>
#include <istream>
#include <ostream>
#include <string>
#include <time.h>

/*Librerie di ROOT*/
#include <TFile.h>
#include <TH1F.h>
#include <TCanvas.h>
#include "TApplication.h"


/*Librerie locali*/
#include "ConfigFile.h"
#include "vme.h"

int A24, A32;
static char *vmedev = VMESTDDEV;

#define CBDBASE ((unsigned long) 0x00800000)

using namespace std;

int
main (int argc, char *argv[])
  {

  // TApplication theApp("App", NULL, NULL);

    
    
    string outfileTDC1,outfileTDC2;
    string outfileADC1,outfileADC2;
    int nev; // numero di conteggi
    int i; // numero di volte che testa il LAM
    int daq_typ=1;
    cout<<"Getting input from: read.inp"<<endl;
    ConfigFile config( "read.inp" ); // crea un file di configurazione
    config.readInto( outfileTDC1, "outfileTDC1" );
    config.readInto( outfileTDC2, "outfileTDC2" );
    config.readInto( outfileADC1, "outfileADC1" );
    config.readInto( outfileADC2, "outfileADC2" );
    config.readInto( nev, "nev" ); // legge il numero di conteggi da fare
    config.readInto ( i, "i" ); // legge il numero di volte che deve testare il LAM
    config.readInto ( daq_typ, "daq_typ" ); // legge il tipo di acqusizione d fare: 0 -> entrambi ADC+TDC, 1->ADC, 2->TDC

    
  /* Configura il VME */     
  if (VMEopen (vmedev, 0x39, &A24)){
    
                                    fputs ("FATAL: VMEopen failed (A24)!\n", stderr);
                                    return 1;
                                   }

  if (VMEopen (vmedev, 0x09, &A32)){
                                    fputs ("FATAL: VMEopen failed (A32)!\n", stderr);
                                    return 1;
                                   }
    
  fputs ("VME initialized successfully.\n", stdout);

  /* Definisce l'indirizzo del CAMAC  */
  int b=0; // Definisce il numero del Brench
  int c=1; // Definisce il numero del Crate
  int n=13; // Definisce il numero dello slot del TDC
  //int m=8; // Definisce il numero dello slot dell'ADC
   int m=11; // Definisce il numero dello slot dell'ADC
 
  
  int qTDC,qADC; // Dichiara le variabili per il LAM del TDC e dell'ADC
  int cont;
    
  printf("Q: %d\n", CamacGetQ(A24,CBDBASE));
  CamacBranchReset(A24,CBDBASE);
  printf("Q: %d\n", CamacGetQ(A24,CBDBASE));
  fprintf(stderr, "Resetting CAMAC crate %d (branch %d).\n", c, b);
  CCCZ(A24, CBDBASE, b, c);
  CCCC(A24, CBDBASE, b, c);
  CCCI(A24, CBDBASE, b, c, 0);
  
  /*Definizione delle variabili dati*/
  unsigned short dataTDC1, dataTDC2, dataTDC3, dataTDC4, dataTDC5, dataTDC6;  
  unsigned short dataADC1, dataADC2, dataADC3, dataADC4, dataADC5, dataADC0;
  time_t seconds;seconds = time (NULL);
  
  int j=1; // Descrive il numero dell'evento   
  
  CSSA_x1(A24,CBDBASE,b,c,n,0,26); //enable LAM TDC
  CSSA_x1(A24,CBDBASE,b,c,n,0,10); //clear LAM TDC
  
  CSSA_x1(A24,CBDBASE,b,c,m,0,26); //enable LAM ADC
  CSSA_x1(A24,CBDBASE,b,c,m,0,10); //clear LAM ADC
    
  /* Crea i files di ROOT */
  TFile *foutTDC = new TFile(outfileTDC1.c_str(),"RECREATE"); // Crea un file di ROOT per il TDC
  TFile *foutADC = new TFile(outfileADC1.c_str(),"RECREATE"); // Crea un file di ROOT per l'ADC
  
  /* Crea gli istogrammi di ROOT */  
  TH1F *TDC1 = new TH1F("tdc1","spettro TDC scintillatore 1",4000,-0.5,3999.5); // Crea un istogramma di ROOT con lo spettro del TDC per lo scintillatore 1
  TH1F *TDC2 = new TH1F("tdc2","spettro TDC scintillatore 2",4000,-0.5,3999.5); // Crea un istogramma di ROOT con lo spettro del TDC per lo scintillatore 2
  TH1F *TDC3 = new TH1F("tdc3","spettro TDC scintillatore 3",4000,-0.5,3999.5); // Crea un istogramma di ROOT con lo spettro del TDC per lo scintillatore 3
  TH1F *TDC4 = new TH1F("tdc4","spettro TDC scintillatore 4",4000,-0.5,3999.5); // Crea un istogramma di ROOT con lo spettro del TDC per lo scintillatore 4
  TH1F *TDC5 = new TH1F("tdc5","spettro TDC scintillatore 5",4000,-0.5,3999.5); // Crea un istogramma di ROOT con lo spettro del TDC per lo scintillatore 5
  TH1F *TDC_p = new TH1F("tdc_p","spettro TDC scintillatore piccolo",4000,-0.5,3999.5); // Crea un istogramma di ROOT con lo spettro del TDC per lo scintillatore piccolo
  
  TH1F *ADC_p = new TH1F("adc_p","spettro canale 0",2000,-0.5,1999.5); // Crea un istogramma di ROOT per lo scintillatore piccolo
  TH1F *ADC1 = new TH1F("adc1","spettro canale 1",2000,-0.5,1999.5); // Crea un istogramma di ROOT con lo spettro del TDC per lo scintillatore 1
  TH1F *ADC2 = new TH1F("adc2","spettro canale 2",2000,-0.5,1999.5); // Crea un istogramma di ROOT con lo spettro del TDC per lo scintillatore 2
  TH1F *ADC3 = new TH1F("adc3","spettro canale 3",2000,-0.5,1999.5); // Crea un istogramma di ROOT con lo spettro del TDC per lo scintillatore 3
  TH1F *ADC4 = new TH1F("adc4","spettro canale 4",2000,-0.5,1999.5); // Crea un istogramma di ROOT con lo spettro del TDC per lo scintillatore 4
  TH1F *ADC5 = new TH1F("adc5","spettro canale 5",2000,-0.5,1999.5); // Crea un istogramma di ROOT con lo spettro del TDC per lo scintillatore 5
  TH1F *sum = new TH1F("sum","spettro somma",2000,-0.5,1999.5); //Crea un oggetto di ROOT che somma gli istogrammi
  
  /* Apre i files di input */    
  ofstream of_TDC;
  of_TDC.open(outfileTDC2.c_str()); // Apre il file ASCII di output per il TDC
  
  ofstream of_ADC;
  of_ADC.open(outfileADC2.c_str()); // Apre il file ASCII di output per il ADC
    
  cout<<"Reading "<<nev<<" events."<<endl; // Scrive il numero di conteggi che fa
  cout<<"Testa il LAM " <<i <<" volte." <<endl;
  
  /* Testa il LAM */ 
  while(j<nev){
               qTDC=0; // imposta il LAM del TDC a 0
               qADC=0; // imposta il LAM dell'ADC a 0
               cont=0;
	       
              // while(qTDC!=1 || qADC!=1){ //richiede entrambi i Q !!!!!!!!!!!1
	        while( (qADC!=1 && daq_typ==1) || (qTDC!=1 && daq_typ==2 )  ||  ((qTDC!=1 || qADC!=1)&&(daq_typ==0 ))  ){  // richiede solo uno dei due Q
	                                 qTDC=CSSA_x1(A24,CBDBASE,b,c,n,0,8); //TEST LAM TDC
                                         qADC=CSSA_x1(A24,CBDBASE,b,c,m,0,8); //TEST LAM ADC
                                         cont++;
					 
					
					//cout<<" cont = "<<cont<<endl;
                                         if(cont==i){
                                                        cout<<"Q not found ("<<i<<" times checked)! Event "<<j<<endl;
	                                                    break;
                                         }
                 }
                
		if(cont==i){
		
			   CSSA_x1(A24,CBDBASE,b,c,n,0,9); //clear tdc  disable LAM 
                           CSSA_x1(A24,CBDBASE,b,c,n,0,10); //clear LAM
                           CSSA_x1(A24,CBDBASE,b,c,n,0,26); //enable LAM
    		        
                           CSSA_x1(A24,CBDBASE,b,c,m,0,9); //clear adc  
                           CSSA_x1(A24,CBDBASE,b,c,m,0,10); //clear LAM
			   CSSA_x1(A24,CBDBASE,b,c,m,0,26); //enable LAM
					
		 
		continue;}
              // if(qADC==1 && qTDC==1) //legge (e scrive) i dati solo se e' stato trovato il LAM in entrambi
               if( (qADC==1 && daq_typ==1) ||(qTDC==1 && daq_typ==2) || (qADC==1 && qTDC==1 && daq_typ==0)   ) // legge (e scrive) i dati solo se e' stato trovato il LAM di uno
                       {
                 	  cout<<"QADC ="<<qADC<<"  qTDC = "<<qTDC<<endl;
		 	    j++;	
		           /*for(int x=0;x<10000000;x++){
    	                           if((CSSA(A24,CBDBASE,b,c,n,0,0,&dataTDC1))==1) //read data TDC 1 e testa se ha finito di convertire i dati
	                           break;
				   }*/
				   CSSA(A24,CBDBASE,b,c,n,0,0,&dataTDC1); //read data TDC 1
	                       	   CSSA(A24,CBDBASE,b,c,n,1,0,&dataTDC2); //read data TDC 2
	                       	   CSSA(A24,CBDBASE,b,c,n,2,0,&dataTDC3); //read data TDC 3
	                       	   CSSA(A24,CBDBASE,b,c,n,3,0,&dataTDC4); //read data TDC 4
	                       	   CSSA(A24,CBDBASE,b,c,n,4,0,&dataTDC5); //read data TDC 5 
				   CSSA(A24,CBDBASE,b,c,n,5,0,&dataTDC6); //read data TDC piccolino  
    	                   	   seconds = time (NULL); //get the time	
    
                           /*for(int x=0;x<10000000;x++){	
                                   if((CSSA(A24,CBDBASE,b,c,m,0,0,&dataADC1))==1) //read data ADC 1 e testa se ha finito di convertire i dati
	                           break;
				   }*/
				   CSSA(A24,CBDBASE,b,c,m,0,0,&dataADC0); //read data ADC piccolino
                           	   CSSA(A24,CBDBASE,b,c,m,1,0,&dataADC1); //read data ADC 1
	                       	   CSSA(A24,CBDBASE,b,c,m,2,0,&dataADC2); //read data ADC 2
	                       	   CSSA(A24,CBDBASE,b,c,m,3,0,&dataADC3); //read data ADC 3
	                       	   CSSA(A24,CBDBASE,b,c,m,4,0,&dataADC4); //read data ADC 4
	                       	   CSSA(A24,CBDBASE,b,c,m,5,0,&dataADC5); //read data ADC 5
    	
	                   if(j%1==0){
                                     cout<<"Event "<<j-1<<", gt:"<<seconds<<", chTDC1:"<<dataTDC1<<", chTDC2:"<<dataTDC2 <<", chTDC3:"<<dataTDC3<<", chTDC4:"<<dataTDC4<<", chTDC5:"<<dataTDC5<<", chTDC6:"<<dataTDC6<<endl;
                                      cout<<"Event "<<j-1<<", chADC1:"<<dataADC1<<", chADC2:"<<dataADC2 <<", chADC3:"<<dataADC3<<", chADC4:"<<dataADC4<<", chADC5:"<<dataADC5<<", chADC6:"<<dataADC0<<endl<<endl;
                                      }  
       
                           TDC_p->Fill(dataTDC6);
                           TDC1->Fill(dataTDC1);
                           TDC2->Fill(dataTDC2);
                           TDC3->Fill(dataTDC3);
                           TDC4->Fill(dataTDC4);
                           TDC5->Fill(dataTDC5);
    
                           ADC_p->Fill(dataADC0);
                           ADC1->Fill(dataADC1);
                           ADC2->Fill(dataADC2);
                           ADC3->Fill(dataADC3);
                           ADC4->Fill(dataADC4);
                           ADC5->Fill(dataADC5);
    
                           sum->Add(ADC1);
                           sum->Add(ADC2);
                           sum->Add(ADC3);
                           sum->Add(ADC4);
                           sum->Add(ADC5);
    
                           of_TDC<<j-1<<" "<<seconds<<" "<<dataTDC1 <<" "<<dataTDC2 <<" "<<dataTDC3 <<" "<<dataTDC4    <<" "<<dataTDC5 <<" "<<dataTDC6 <<endl;
    
                           of_ADC<<j-1<<" "<<dataADC1<<" "<<dataADC2<<" "<<dataADC3<<" "<<dataADC4<<" "<<dataADC5<<" "<<dataADC0<<endl;
    
    
    
    
    
    
    		             CSSA_x1(A24,CBDBASE,b,c,n,0,9); //clear tdc  disable LAM 
                           CSSA_x1(A24,CBDBASE,b,c,n,0,10); //clear LAM
                           CSSA_x1(A24,CBDBASE,b,c,n,0,26); //enable LAM
    		        
                           CSSA_x1(A24,CBDBASE,b,c,m,0,9); //clear adc  
                           CSSA_x1(A24,CBDBASE,b,c,m,0,10); //clear LAM
			//   CSSA_x1(A24,CBDBASE,b,c,m,0,26); //enable LAM
    
     		           }
              }
  
    VmeWriteShort(A24,(CBDBASE)|(CBD8210_CAR),0x5555);
    printf("CAR: %04x\n",VmeReadShort(A24,CBDBASE|(CBD8210_CSR)));
    
    /* Imposta i colori per gli istogrammi */
    TDC1->SetLineColor(8); // colora lo spettro di verde
    TDC2->SetLineColor(9); // colora lo spettro di blu
    TDC3->SetLineColor(6); // colora lo spettro di fucsia
    TDC4->SetLineColor(50); // colora lo spettro di rosso
    TDC5->SetLineColor(41); // colora lo spettro di marroncino
    
    ADC1->SetLineColor(8); // colora lo spettro di verde
    ADC2->SetLineColor(9); // colora lo spettro di blu
    ADC3->SetLineColor(6); // colora lo spettro di fucsia
    ADC4->SetLineColor(50); // colora lo spettro di rosso
    ADC5->SetLineColor(41); // colora lo spettro di marroncino
        
    /* Disegna gli istogrammi degli spettri per il TDC */
    foutTDC->cd(); // seleziona il file "foutTDC" come quello su cui scrivere
    
    TCanvas *plotTDC = new TCanvas("tdc","Picchi TDC",0); // Crea un Canvas di ROOT
    
    TDC1->Draw(); // disegna lo spettro del canale 1
    TDC2->Draw("same"); // sovrappone lo spettro del canale 2 a quello precedente
    TDC3->Draw("same"); // sovrappone lo spettro del canale 3 a quelli precedenti
    TDC4->Draw("same"); // sovrappone lo spettro del canale 4 a quelli precedenti
    TDC5->Draw("same"); // sovrappone lo spettro del canale 5 a quelli precedenti
    TDC_p->Draw("same"); // sovrappone lo spettro del piccolino a quelli precedenti
    
    plotTDC->Write();  // Scrive il Canvas "plotTDC"
    
    TDC1->Write(); // scrive l'istogramma dello spettro dello scintillatore 1
    TDC2->Write(); // scrive l'istogramma dello spettro dello scintillatore 2
    TDC3->Write(); // scrive l'istogramma dello spettro dello scintillatore 3
    TDC4->Write(); // scrive l'istogramma dello spettro dello scintillatore 4
    TDC5->Write(); // scrive l'istogramma dello spettro dello scintillatore 5
    TDC_p->Write(); // scrive l'istogramma dello spettro dello scintillatore piccolo
        
    foutTDC->Close(); // Chiude il file di output del TDC
    plotTDC->Close(); // Chiude il Canvas "plotTDC"
    of_TDC.close(); // Chiude il file ASCII di output per il TDC
    
    /* Disegna gli istogrammi degli spettri per l'ADC */
    foutADC->cd(); // seleziona il file "foutADC" come quello su cui scrivere
    
    TCanvas *plotADC = new TCanvas("adc","Picchi ADC",0); // Crea un Canvas di ROOT
    
    ADC1->Draw(); // disegna lo spettro del canale 1
    ADC2->Draw("same"); // sovrappone lo spettro del canale 2 a quello precedente
    ADC3->Draw("same"); // sovrappone lo spettro del canale 3 a quelli precedenti
    ADC4->Draw("same"); // sovrappone lo spettro del canale 4 a quelli precedenti
    ADC5->Draw("same"); // sovrappone lo spettro del canale 5 a quelli precedenti
    ADC_p->Draw("same"); // sovrappone lo spettro del piccolino a quelli precedenti
    sum->Draw(); // disegna il grafico della somma degli spettri
    
    plotADC->Write(); // Scrive il Canvas "plotADC"
    
    ADC1->Write(); // scrive l'istogramma dello spettro dello scintillatore 1
    ADC2->Write(); // scrive l'istogramma dello spettro dello scintillatore 2
    ADC3->Write(); // scrive l'istogramma dello spettro dello scintillatore 3
    ADC4->Write(); // scrive l'istogramma dello spettro dello scintillatore 4
    ADC5->Write(); // scrive l'istogramma dello spettro dello scintillatore 5
    ADC_p->Write(); // scrive l'istogramma dello spettro dello scintillatore piccolo
    sum->Write();
    
    foutADC->Close(); // Chiude il file di output del ADC
    plotADC->Close(); // Chiude il Canvas "plotADC"
    of_ADC.close(); // Chiude il file ASCII di output per l'ADC
    
  return 0;
}
