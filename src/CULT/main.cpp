#include <iostream>
#include <cmath>

//éléments définissant l'interface et les options de chorégraphie (timer, pas de détection...)

#define WIDTH 640
#define HEIGHT 480

#define LAUNCHEDZ 1.00

#define DIV 3

#define RED Scalar(0.1, 0.2, 0.3)
#define GREEN Scalar(0.1, 0.2, 0.3)

#define VROCK 0
#define VELECTRO 1
#define VHIPHOP 2

#define VPENCIL 0
#define VBRUSH 1

#define VSAVE 13

#define STYLE_TRESHOLD 10
#define TOOL_TRESHOLD 10
#define SAVE_TRESHOLD 5

#define MARGIN 15.

#define COLORCHANGE 3

// tuio
#include "TuioServer.h"

// opencv
#pragma comment(lib, "cxcore210d.lib")
#pragma comment(lib, "cv210d.lib")
#pragma comment(lib, "highgui210d.lib")
#include <opencv/highgui.h>
#include <opencv/cv.h>

#include "ChromaticPoint.h"
#include "cultWindow.h"

// kinect
#include <CLNUIDevice.h>
#pragma comment(lib, "CLNUIDevice.lib")

cultWindow * output;
enum types {STYLE, TOOL, SAVE};

//traite les points envoyés par CLNUI
void unproject(unsigned short* depth, float* x, float* y, float* z) {
	int u,v;
	const float f = 500.0f;
	const float u0 = 320.0f;
	const float v0 = 240.0f;
	float zCurrent;

	// TODO calibration

	for (int i=0; i<640*480; i++) {
		u = i % 640;
		v = i / 640;
		zCurrent = 1.0f / (-0.00307110156374373f * depth[i] + 3.33094951605675f);
		if (z != NULL) {
			z[i] = zCurrent;
		}
		if (x != NULL) {
			x[i] = (u - u0) * zCurrent / f;
		}
		if (y != NULL) {
			y[i] = (v - v0) * zCurrent / f;
		}
	}
}

//établit la moyenne entre les différentes gstures détectées et en extrait la plus probable (celle qu'avait voulu l'utilisateur)
void majority(int type)
{
	double maj = 0.;
	for(int i = 0 ; i < output->Codes.size() ; i++)
		maj += output->Codes[i];

	maj = (double)maj / output->Codes.size();

	//std::cout << "MAJ : " << maj << std::endl;

	if(type == types::STYLE)
	{
		if(maj >= 0. && maj <= 0.3)
			output->setStyle(output->ROCK);
		else if(maj > 0.3 && maj < 1.3)
			output->setStyle(output->ELECTRO);
		else if(maj <= 2.0)
			output->setStyle(output->HIPHOP);
		output->incrementColor();
		output->hiphopCount = 1;
		output->rockCount = 1;
		output->electroCount = 1;
	}
	else if(type == types::TOOL)
	{
		if(maj >= 0. && maj < 0.5)
			output->setTools(output->PENCIL);
		else if(maj < 2.)
			output->setTools(output->BRUSH);
		output->pencilCount = 1;
		output->brushCount = 1;
	}
	else if(type = types::SAVE)
	{
		if(maj > 12. && maj < 14.)
		{
			output->showCult();
			output->save();
		}
		output->saveCount = 1;
	}
}

//renverse l'affichage de manière à avoir un effet miroir. NB : la caméra n'est pas inversée par soucis de performances.
cv::Point reverse(cv::Point pt)
{
	cv::Point ret(WIDTH - pt.x, pt.y);
	return ret;
}

//distance euclidienne entre deux points
double dist(cv::Point a, cv::Point b)
{
	return cv::sqrt((double)((a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y)));
}

//fonction de tracking d'une gesture à 3 doigts (HIP HOP / ELECTRO)
void recon3(std::vector<double> approxDist, cv::Point rightCenter)
{
	using namespace cv;
	using namespace std;

	static int calc = 0;
	static double d1hh = 0.;
	static double d2hh = 0.;
	static double d3hh = 0.;
	static double d1e = 0.;
	static double d2e = 0.;
	static double d3e = 0.;

	int size = approxDist.size();
	sort(approxDist.begin(), approxDist.end());

	if(output->calibrating)
	{
		if(output->calType == output->CHIPHOP)
		{
			d1hh += approxDist[size-3];
			d2hh += approxDist[size-2];
			d3hh += approxDist[size-1];
			calc++;
	
			if(calc > 100)
			{
				d1hh /= calc;
				d2hh /= calc;
				d3hh /= calc;

				/*cout << d1hh << endl;
				cout << d2hh << endl;
				cout << d3hh << endl;*/
				/*cin >> calc;
				if(calc < 100)
				{
					d1hh = 0.;
					d2hh = 0.;
					d3hh = 0.;
				}*/
				output->calHipHop.push_back(d3hh);
				output->calHipHop.push_back(d2hh);
				output->calHipHop.push_back(d1hh);
				output->calProgress++;
				calc = 0;
				d1hh = 0.;
				d2hh = 0.;
				d3hh = 0.;
			}
		}
		else if(output->calType == output->CELECTRO)
		{
			d1e += approxDist[size-3];
			d2e += approxDist[size-2];
			d3e += approxDist[size-1];
			calc++;
	
			if(calc > 100)
			{
				d1e /= calc;
				d2e /= calc;
				d3e /= calc;

				/*cout << d1e << endl;
				cout << d2e << endl;
				cout << d3e << endl;*/
				/*cin >> calc;
				if(calc < 100)
				{
					d1hh = 0.;
					d2hh = 0.;
					d3hh = 0.;
				}*/
				output->calElectro.push_back(d3e);
				output->calElectro.push_back(d2e);
				output->calElectro.push_back(d1e);
				output->calProgress++;
				calc = 0;
				d1e = 0.;
				d2e = 0.;
				d3e = 0.;
			}
		}
	}

	if(!output->calibrating)
	{
		if(approxDist[size-3] > output->calHipHop[0] - MARGIN && approxDist[size-3] < output->calHipHop[0] + MARGIN && approxDist[size-2] > output->calHipHop[1] - MARGIN && approxDist[size-2] < output->calHipHop[1] + MARGIN && approxDist[size-1] > output->calHipHop[2] - MARGIN && approxDist[size-1] < output->calHipHop[2] + MARGIN)
		{
			cout << "HIP HOP (" << output->hiphopCount++ << "/" << STYLE_TRESHOLD << ")" << endl;
			if(output->noStyle())
				output->Codes.push_back(VHIPHOP);
		}

		if(approxDist[size-3] > output->calElectro[0] - MARGIN && approxDist[size-3] < output->calElectro[0] + MARGIN && approxDist[size-2] > output->calElectro[1] - MARGIN && approxDist[size-2] < output->calElectro[1] + MARGIN && approxDist[size-1] > output->calElectro[2] - MARGIN && approxDist[size-1] < output->calElectro[2] + MARGIN)
		{
			cout << "ELECTRO (" << output->electroCount++ << "/" << STYLE_TRESHOLD << ")" << endl;
			if(output->noStyle())
				output->Codes.push_back(VELECTRO);
		}
	}

	/*for(int i = 0 ; i < approxDist.size() ; i++)
		cout << approxDist[i] << endl;
	cout << endl << endl;*/
	//calc++;
}

//fonction de comparaison pour les points de type chromatic circle, utilisés pour faire varier la couleur 
bool comp(ChromaticPoint pt1, ChromaticPoint pt2)
{
	//return (pt1.dist < pt2.dist) ? pt1 : pt2;
	return pt1.getDist() < pt2.getDist();
}

//fonction de comparaison pour récupérer par la suite le point le plus haut
bool getHigher(cv::Point p1, cv::Point p2)
{
	return p1.y < p2.y;
}

//récupère la position du curseur
void getCursor(std::vector<cv::Point> pts, std::vector<int> hull, cv::Mat1f z)
{
	using namespace cv;
	using namespace std;

	static Point curBefore = Point(-1., -1.);

	int size = pts.size();

	sort(pts.begin(), pts.end(), getHigher);

	Point curNow = Point(pts[0].x*2-WIDTH*2/DIV,pts[0].y*2);
	//for(int i = 0 ; i < pts.size() ; i++)
		//cout << pts[0].y << endl;

	//system("PAUSE");
		
		//s'il se situe sur la partie de droite de l'écran :
		if(pts[size-1].x >= WIDTH/DIV)// && (curBefore.x == -1. && curBefore.y == -1 || abs(curBefore.x - pts[0].x) <= 10 && abs(curBefore.y - pts[0].y) <= 10))
		{
			if(curBefore != Point(-1., -1.))
				output->draw(curBefore, curNow);
			//cout << curBefore.x << ";" << curBefore.y << " => " << pts[0].x << ";" << pts[0].y << endl;
			curBefore = curNow;
			output->setCursor(curNow);
		}
	//cout << "(" << pts[size-1].x << ";" << pts[size-1].y << ")(" << z.depth() << ")" << endl;
}

//fonction de reconnaissance du crayon
void reconPencil(std::vector<double> approxDist, cv::Point rightCenter)
{
	using namespace cv;
	using namespace std;

	static int calc = 0;
	static double d1p = 0.;

	int size = approxDist.size();
	sort(approxDist.begin(), approxDist.end());

	if(output->calibrating)
	{
		if(output->calType == output->CPENCIL)
		{
			d1p += approxDist[size-1];
			calc++;

			//cout << calc << endl;
	
			if(calc > 100)
			{
				d1p /= calc;

				//cout << d1p << endl;
				/*cin >> calc;
				if(calc < 100)
				{
					d1hh = 0.;
					d2hh = 0.;
					d3hh = 0.;
				}*/
				output->calPencil.push_back(d1p);
				output->calProgress++;
				calc = 0;
				d1p = 0.;
			}
		}
	}

	if(!output->calibrating)
	{
		if(approxDist[size-1] > output->calPencil[0] - MARGIN && approxDist[size-1] < output->calPencil[0] + MARGIN)
		{
			cout << "PENCIL (" << output->pencilCount++ << "/" << TOOL_TRESHOLD << ")" << endl;
			output->Codes.push_back(VPENCIL);
			if(!output->noStyle() &&  output->Codes.size() == 1)
				output->waitFlush(5., true);
		}
	}
}

//fonction de reconnaissance du pinceau
void reconBrush(std::vector<double> approxDist, cv::Point rightCenter)
{
	using namespace cv;
	using namespace std;

	int size = approxDist.size();
	sort(approxDist.begin(), approxDist.end());
	
	//if(approxDist[size-2] > 110 && approxDist[size-2] < 130 && approxDist[size-1] > 110 && approxDist[size-1] < 125)
	//{
		cout << "BRUSH (" << output->brushCount++ << "/" << TOOL_TRESHOLD << ")" << endl;
		output->Codes.push_back(VBRUSH);
	//}
}

//fonction de modification de la couleur courante par reconnaissance de la main gauche dans la partie gauche
//(soucis d'implémentation du tracking de "rotation", donc traité comme une fonction recon sur 5 doigts de la main gauche, en se basant 
//également sur les distances entre centroide de la main gauche et les deux doigts détectés les plus longs)
void chromaticCircle(std::vector<ChromaticPoint> approxPts, cv::Point leftCenter)
{
	using namespace cv;
	using namespace std;

	static int calc = 0;
	static double d1c = 0.;
	static double d2c = 0.;

	int size = approxPts.size();
	sort(approxPts.begin(), approxPts.end(), comp);
	
	//Point p1 = approxPts[size-2].getPt();
	//Point p2 = approxPts[size-1].getPt();

	//int a1 = (int)(360 * acos(abs(leftCenter.y - p1.y) / d1) / 2 * 3.14)%180;
	//int a2 = (int)(360 * acos(abs(leftCenter.y - p2.y) / d2) / 2 * 3.14)%180;

	//cout << "chromatic" << endl;

	if(output->calibrating)
	{
		if(output->calType == output->CCOLOR)
		{
			d1c += approxPts[size-2].getDist();
			d2c += approxPts[size-1].getDist();
			calc++;
	
			if(calc > 100)
			{
				d1c /= calc;
				d2c /= calc;

				/*cout << d1c << endl;
				cout << d2c << endl;*/
				/*cin >> calc;
				if(calc < 100)
				{
					d1hh = 0.;
					d2hh = 0.;
					d3hh = 0.;
				}*/
				output->calColor.push_back(d2c);
				output->calColor.push_back(d1c);
				output->calProgress++;
				calc = 0;
				d1c = 0.;
				d2c = 0.;
			}
		}
	}

	if(!output->calibrating)
	{
		double d1 = approxPts[size-2].getDist();
		double d2 = approxPts[size-1].getDist();

		/*for(int i = approxPts.size()-3 ; i < approxPts.size() ; i++)
			cout << approxPts[i].getDist() << " [" << approxPts[i].getPt().x << ";" << approxPts[i].getPt().y << "]" << " - [" 
			<< leftCenter.x << ";" << leftCenter.y << "]" << " A = " << (int)(360 * acos(abs(leftCenter.y - approxPts[i].getPt().y) / approxPts[i].getDist()) / 2 * 3.14)%180 << endl;*/

		if(d1 > output->calColor[0] - MARGIN && d1 < output->calColor[0] + MARGIN && d2 > output->calColor[1] - MARGIN && d2 < output->calColor[1] + MARGIN)
		{
			cout << "CHANGE COLOR (" << output->colorCount++ << "/" << COLORCHANGE << ")" << endl;
			if(output->colorCount > COLORCHANGE)
			{
				output->colorCount = 1;
				output->incrementColor();
			}
		}
	}
}

//fonction de reconnaissance d'une gesture à 2 doigts (rock)
void recon2(std::vector<double> approxDist, cv::Point rightCenter)
{
	using namespace cv;
	using namespace std;

	static int calc = 0;
	static double d1r = 0.;
	static double d2r = 0.;

	int size = approxDist.size();
	sort(approxDist.begin(), approxDist.end());

	if(output->calibrating)
	{
		if(output->calType == output->CROCK)
		{
			d1r += approxDist[size-2];
			d2r += approxDist[size-1];
			calc++;
	
			if(calc > 100)
			{
				d1r /= calc;
				d2r /= calc;

				/*cout << d1r << endl;
				cout << d2r << endl;*/
				/*cin >> calc;
				if(calc < 100)
				{
					d1hh = 0.;
					d2hh = 0.;
					d3hh = 0.;
				}*/
				output->calRock.push_back(d2r);
				output->calRock.push_back(d1r);
				output->calProgress++;
				calc = 0;
				d1r = 0.;
				d2r = 0.;
			}
		}
	}

	if(!output->calibrating)
	{
		if(approxDist[size-2] > output->calRock[0] - MARGIN && approxDist[size-2] < output->calRock[0] + MARGIN && approxDist[size-1] > output->calRock[1] - MARGIN && approxDist[size-1] < output->calRock[1] + MARGIN)
		{
			cout << "ROCK (" << output->rockCount++ << "/" << STYLE_TRESHOLD << ")" << endl;
			if(output->noStyle())
				output->Codes.push_back(VROCK);
		}
	}

	/*for(int i = 0 ; i < approxDist.size() ; i++)
		cout << approxDist[i] << endl;
	cout << endl << endl;*/
	//calc++;
}

//fonctionne de la meme manière que rock, mais pour sauvegarder (sur la partie gauche de l'écran)
void recon2save(std::vector<double> approxDist, cv::Point leftCenter)
{
	using namespace cv;
	using namespace std;

	static int calc = 0;
	static double d1s = 0.;
	static double d2s = 0.;

	int size = approxDist.size();
	sort(approxDist.begin(), approxDist.end());

	if(output->calibrating)
	{
		if(output->calType == output->CSAVE)
		{
			d1s += approxDist[size-2];
			d2s += approxDist[size-1];
			calc++;
	
			if(calc > 100)
			{
				d1s /= calc;
				d2s /= calc;

				/*cout << d1s << endl;
				cout << d2s << endl;*/
				/*cin >> calc;
				if(calc < 100)
				{
					d1hh = 0.;
					d2hh = 0.;
					d3hh = 0.;
				}*/
				output->calSave.push_back(d2s);
				output->calSave.push_back(d1s);
				output->calProgress++;
				calc = 0;
				d1s = 0.;
				d2s = 0.;
			}
		}
	}

	if(!output->calibrating)
	{
		if(approxDist[size-2] > output->calSave[0] - MARGIN && approxDist[size-2] < output->calSave[0] + MARGIN && approxDist[size-1] > output->calSave[1] - MARGIN && approxDist[size-1] < output->calSave[1] + MARGIN)
		{
			cout << "SAVE (" << output->saveCount++ << "/" << SAVE_TRESHOLD << ")" << endl;
				output->Codes.push_back(VSAVE);
		}
	}

	//calc++;
}

//fonction de détection des doigts (DEJA PRESENTE mais modifiée de manière à remplir les besoins du prototype), elle permet à partir d'une forme dont la surface dépasse la valeur 1000 d'en extraire le contour, et
//de savoir si un doigt est levé ou non.
std::vector<cv::Point2i> detectFingertips(cv::Mat1f z, float zMin = 0.0f, float zMax = 0.75f, cv::Mat1f& debugFrame = cv::Mat1f()) {
	using namespace cv;
	using namespace std;
	bool debug = !debugFrame.empty();

	output->wait(2., false);
	output->waitFlush(5., false);
	//centroide (pour une surface donnée) corrigé (plus centré dans la surface)
	Point centerPointCorrected;

	vector<Point2i> fingerTips;

	Mat handMask = z < zMax & z > zMin;

	std::vector<std::vector<cv::Point> > contours;
	findContours(handMask.clone(), contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE); // we are cloning here since method will destruct the image

	line(debugFrame, Point(WIDTH/DIV, 0.), Point(WIDTH/DIV, HEIGHT), Scalar(1), 3);

	if (contours.size()) {
		for (int i=0; i<contours.size(); i++) {
			vector<Point> contour = contours[i];
			Mat contourMat = Mat(contour);
			double area = cv::contourArea(contourMat);

			if (area > 1000)  { // forme pouvant etre considérée dans les conditions du prototype par une main
				Scalar center = mean(contourMat);
				Point centerPoint = Point(center.val[0], center.val[1]);

				/////////////////////

				//cv::circle(debugFrame, centerPoint, 20, Scalar(1), -1);

				/////////////////////


				//points qui seront détectés (comme doigts ou non)
				vector<Point> approxCurve;
				cv::approxPolyDP(contourMat, approxCurve, 20, true);

				for(int a = 0 ; a < approxCurve.size() ; a++)
				{
					approxCurve[a] = reverse(approxCurve[a]);
				}

				vector<int> hull;
				cv::convexHull(Mat(approxCurve), hull);

				//troube les limites supérieures et inférieurs de la main et définit le seuil de coupure (les vertices basses ne seront pas considérés comme des doigts)
				int upper = 640, lower = -100; ///////// lower = 100
				for (int j=0; j<hull.size(); j++) {
					int idx = hull[j]; // corner index
					if (approxCurve[idx].y < upper) upper = approxCurve[idx].y;
					if (approxCurve[idx].y > lower) lower = approxCurve[idx].y;
				}
				float cutoff = lower - (lower - upper) * 0.1f;

				int countRight = 0;
				int countLeft = 0;
				vector<double> reconDistRight(approxCurve.size(), -1.);
				vector<double> reconDistLeft(approxCurve.size(), -1.);
				vector<ChromaticPoint> chromaticLeftHand(approxCurve.size());

				// find interior angles of hull corners
				for (int j=0; j<hull.size(); j++) {
					int idx = hull[j]; // corner index
					int pdx = idx == 0 ? approxCurve.size() - 1 : idx - 1; //  predecessor of idx
					int sdx = idx == approxCurve.size() - 1 ? 0 : idx + 1; // successor of idx

					Point v1 = approxCurve[sdx] - approxCurve[idx];
					Point v2 = approxCurve[pdx] - approxCurve[idx];

					float angle = acos( (v1.x*v2.x + v1.y*v2.y) / (norm(v1) * norm(v2)) );

					// low interior angle + within upper 90% of region -> we got a finger
					if (angle < 1 && approxCurve[idx].y < cutoff) {
						int u = approxCurve[idx].x;
						int v = approxCurve[idx].y;

						//////CENTRE CORRIGE//////

						centerPointCorrected = centerPoint;
						for(int p = 0 ; p < approxCurve.size() ; p++)
							centerPointCorrected += approxCurve[p];

						centerPointCorrected.x /= (approxCurve.size()+1);
						centerPointCorrected.y /= (approxCurve.size()+1);

						if(centerPointCorrected.x > WIDTH/DIV)
							cv::circle(debugFrame, centerPointCorrected, 20, Scalar(1), -1);
						else
							cv::circle(debugFrame, centerPointCorrected, 20, Scalar(1), -1);

						//////////////////////////

						//si la main est à droite
						if(approxCurve[idx].x >= WIDTH/DIV)
						{
							//on compte le nombre de doigts détectés (levés), puis on ajoute les distances doigts-centroide à un vecteur
							countRight++;
							reconDistRight[idx] = dist(approxCurve[idx], centerPointCorrected);
						}

						//si la main est à gauche
						if(approxCurve[idx].x < WIDTH/DIV)
						{
							//on compte les doigts détectés
							countLeft++;
							//idem pour la main gauche
							reconDistLeft[idx] = dist(approxCurve[idx], centerPointCorrected);
							//idem mais en utilisant la classe cercle chromatique
							chromaticLeftHand[idx].setDist(dist(approxCurve[idx], centerPointCorrected));
							chromaticLeftHand[idx].setPt(approxCurve[idx]);
						}

						//on insère les points détectés dans le vecteur fingertips (pour l'interface debug aidant l'utilisateur à se repérer)
						fingerTips.push_back(Point2i(u,v));
						
						if (debug) {
							//approxCurve[idx] = reverse(approxCurve[idx]);////////////////////////////////
							cv::circle(debugFrame, approxCurve[idx], 10, Scalar(1), -1);
						}
					}
				}


				//cout << count << endl;
				
				/*if(countRight > 0)
					output->setAllowDraw(true);
				else
					output->setAllowDraw(false);*/

				

				//si aucun style n'a été détecté
				if(output->noStyle() || output->calibrating)
				{
					//si 3 doigts à droite
					if(countRight == 3)
						recon3(reconDistRight, centerPointCorrected);
					//si deux à droite
					if(countRight == 2)
						recon2(reconDistRight, centerPointCorrected);
					//si le vecteur de codes de gestures est plein, alors on effectue la moyenne pour décider de la gesture majoritaire (celle qu'à voulu l'utilisateur)
					if(output->Codes.size() >= STYLE_TRESHOLD)
						majority(types::STYLE);
				}
				if(!output->noStyle() || output->calibrating)
				{
					//si un style a été chargé
					//si deux doigts à gauche ==> fonction de sauvegarde
					if(countLeft == 2)
						recon2save(reconDistLeft, centerPointCorrected);
					//si 5 à droite ==> pinceau
					if(countRight == 5)
						reconBrush(reconDistRight, centerPointCorrected);
					//si 1 à droite ==> crayon
					if(countRight == 1)
						reconPencil(reconDistRight, centerPointCorrected);
					//si 5 à gauche ==> on change la couleur
					if(countLeft == 5)
						chromaticCircle(chromaticLeftHand, centerPointCorrected);
					//si le vecteur de codes de gestures est plein, on effectue la moyenne
					if(output->Codes.size() >= TOOL_TRESHOLD)
						majority(types::TOOL);
					else if(output->Codes.size() >= SAVE_TRESHOLD)
						majority(types::SAVE);
				}

				//on réinitialise le comptage des doigts
				countRight = 0;
				countLeft = 0;

				if (debug) {
					// draw cutoff threshold
					cv::line(debugFrame, Point(center.val[0]-100, cutoff), Point(center.val[0]+100, cutoff), Scalar(1.0f));

					// draw approxCurve
					for (int j=0; j<approxCurve.size(); j++) {
						//approxCurve[j] = reverse(approxCurve[j]);
						cv::circle(debugFrame, approxCurve[j], 10, Scalar(1.0f));
						if (j != 0) {
							cv::line(debugFrame, approxCurve[j], approxCurve[j-1], Scalar(1.0f));
						} else {
							cv::line(debugFrame, approxCurve[0], approxCurve[approxCurve.size()-1], Scalar(1.0f));
						}
					}

					

					// draw approxCurve hull
					for (int j=0; j<hull.size(); j++) {
						//approxCurve[hull[j]] = reverse(approxCurve[hull[j]]);
						cv::circle(debugFrame, approxCurve[hull[j]], 10, Scalar(1.0f), 3);
						getCursor(approxCurve, hull, z);
						if(j == 0) {
							cv::line(debugFrame, approxCurve[hull[j]], approxCurve[hull[hull.size()-1]], Scalar(1.0f));
						} else {
							cv::line(debugFrame, approxCurve[hull[j]], approxCurve[hull[j-1]], Scalar(1.0f));
						}
					}
				}
			}
		}
	}

	if(cvWaitKey(1) == 109)
		cout << "CURSEUR : (" << output->cursor.x << " ; " << output->cursor.y << ")" << endl;

	if(cvWaitKey(1) == 99 && !output->calibrating || output->calibrating)
	{
		if(!output->calibrating)
			cout << "Calibrage en cours..." << endl;
		output->calibrating = true;
		output->save();
		//cout << "prog : " << output->calProgress << endl;
		if(output->calProgress == 0)
		{
			output->calPencil.clear();
			output->showTool(output->PENCIL);
			output->calType = output->CPENCIL;
		}
		else if(output->calProgress == 1)
		{
			output->calSave.clear();
			output->showCult();
			output->calType = output->CSAVE;
		}
		else if(output->calProgress == 2)
		{
			output->calRock.clear();
			output->showStyle(output->ROCK);
			output->calType = output->CROCK;
		}
		else if(output->calProgress == 3)
		{
			output->calColor.clear();
			output->showTool(output->BRUSH);
			output->calType = output->CCOLOR;
		}
		else if(output->calProgress == 4)
		{
			output->calElectro.clear();
			output->showStyle(output->ELECTRO);
			output->calType = output->CELECTRO;
		}
		else if(output->calProgress == 5)
		{
			output->calHipHop.clear();
			output->showStyle(output->HIPHOP);
			output->calType = output->CHIPHOP;
		}
		else if(output->calProgress == 6)
		{
			output->launchCal();
			output->calibrating = false;
			output->calProgress = 0;
			output->calType = output->NONCAL;
		}
	}

	return fingerTips;
}



int main(int argc, char **argv) {
	using namespace std;
	using namespace cv;
	using namespace TUIO;

	cv::Mat depthFrameRaw(HEIGHT, WIDTH, CV_16UC1);
	cv::Mat x(HEIGHT, WIDTH, CV_32FC1);
	cv::Mat y(HEIGHT, WIDTH, CV_32FC1);
	cv::Mat z(HEIGHT, WIDTH, CV_32FC1);

	Mat1f debugFrame(HEIGHT, WIDTH);

	//on crée une nouvelle instance de la camera via le SDK CLNUI
	PCHAR serial = GetNUIDeviceSerial(0);
	CLNUICamera cam = CreateNUICamera(serial);

	const float rMin = 25;

	stringstream buffer;

	unsigned short* depthFrameRawData = (unsigned short*) depthFrameRaw.data;
	int key;

	//on démarre le serveur TUIO
	bool tuio3dMode = false;
	TuioServer* tuio = new TuioServer(tuio3dMode);
	TuioTime time;

	bool debug = true;

	float zMin = 0.0f;
	float zMax = 0.75f;

	//on démarre la caméra
	if ( StartNUICamera(cam) ) {
		cout << "cam started" << endl;
		output = new cultWindow();
		cultWindow cacher = new cultWindow(true);
		while ( (key = cvWaitKey(1)) != 27) {
			if ( GetNUICameraDepthFrameRAW(cam, (PUSHORT)depthFrameRaw.data) ) {
				//on extrait les données via CLNUI, puis on les traite avec la fonction unproject
				unproject(depthFrameRawData, (float*)x.data, (float*)y.data, (float*)z.data);
				debugFrame = z * 0.0; 

				if (key == 32) {
					debug = !debug;
				}

				//vecteur contenant les points détectés (doigts ou non)
				std::vector<cv::Point2i> fingerTips;

				if (debug) {					
					fingerTips = detectFingertips(z, 0, LAUNCHEDZ, debugFrame);
				} else {
					// find fingertips
					fingerTips = detectFingertips(z);

					// draw fingetips
					for(vector<Point2i>::iterator it = fingerTips.begin(); it != fingerTips.end(); it++) {
						circle(debugFrame, (*it), 10, Scalar(1.0f), -1);
					}					
				}

				// send fingertip positions via TUIO
				time = TuioTime::getSessionTime();
				tuio->initFrame(time);
				for(vector<Point2i>::iterator it = fingerTips.begin(); it != fingerTips.end(); it++) {
					// pixel coordinates
					int u = (int)(*it).x;
					int v = (int)(*it).y;

					float pX = 1.0f - u / 640.0f;
					float pY = v / 480.0f;
					float pZ = (z.at<float>(v,u) - zMin) / (zMax - zMin);

					//cout << "[" << pX << ";" << pY << ";" << pZ << "]" << endl;

					//donne les points au serveur, qui les met à jour
					TUIO::TuioCursor* cursor = tuio->getClosestTuioCursor(pX,pY,pZ);
					if (cursor == NULL || cursor->getDistance(pX,pY,pZ) > 0.05) {
						
						tuio->addTuioCursor(pX,pY,pZ);

					} else if(cursor->getTuioTime() != time) {
						tuio->updateTuioCursor(cursor, pX, pY, pZ);
					}
				}
				//enlève les points ne correspondant plus à des points détectés
				tuio->stopUntouchedMovingCursors();				
				tuio->removeUntouchedStoppedCursors();
				tuio->commitFrame();

				//cout << tuio->getTuioCursors().size() << endl;
				
				// draw our debugframe
				imshow("debugFrame", debugFrame);
			}
		}
		
	} else {
		cout << "could not start cam" << endl;
	}

	StopNUICamera(cam);

	return EXIT_SUCCESS;
}