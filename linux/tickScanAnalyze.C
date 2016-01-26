{
  gROOT->SetStyle("Plain");
  gStyle->SetPalette(51);
  gStyle->SetNumberContours(2);
  gStyle->SetOptTitle(0);
  gStyle->SetOptStat(0);

  TTree* t = new TTree("t", "t");
  t->SetMarkerStyle(kFullSquare);
  t->SetMarkerSize(0.8);
  t->ReadFile("sn_tester.txt", "Tstart:DeltaT:status", ',');

  TCanvas* cTickScan = new TCanvas("tickScan", "tickScan");

  t->Draw("DeltaT:Tstart:status", "", "colz");
  htemp->GetYaxis()->SetTitle("#DeltaT");

  TCanvas* cGoodEndTimes = new TCanvas("goodEndTimes", "goodEndTimes");
  t->Draw("Tstart+DeltaT","status==1");
  htemp->GetYaxis()->SetTitle("Entries");
  htemp->GetXaxis()->SetTitle("Tend = Tstart + #DeltaT");

}
