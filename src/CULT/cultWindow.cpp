#include "cultWindow.h"

using namespace cv;
using namespace std;

//attends s secondes, puis rend la main à l'écran principal (busy = false)
void cultWindow::wait(double s, bool init)
{
	static clock_t begin = clock();
	clock_t end;
	double secs = 0.;

	if(init)
		begin = clock();

	if(busy)
	{
		end = clock();
		secs = ((double)end - begin) / CLOCKS_PER_SEC;

		if(secs > s)
		{
			busy = false;
			cvShowImage("CULT", pixels);
			//si un outil est déjà détecté, alors on peut dessiner
			if(currentTool != Tools::NONET)
				allowDraw = true;
		}
	}
}

//permet de vider le vecteur de codes de gestures toutes les s secondes, de facon à limiter au maximum les faux positifs
void cultWindow::waitFlush(double s, bool init)
{
	static clock_t beg = clock();
	clock_t end;
	double secs = 0.;

	if(init)
		beg = clock();

	end = clock();
	secs = ((double)end - beg) / CLOCKS_PER_SEC;

	if(secs > s)
	{
		beg = clock();
		//cout << "REINITIALIZED : " << this->Codes.size() << endl;
		brushCount = 1;
		pencilCount = 1;
		rockCount = 1;
		electroCount = 1;
		hiphopCount = 1;
		saveCount = 1;
		colorCount = 1;
		this->Codes.clear();
	}
}

//calcule la distance euclidienne entre deux points 2D
double getDist(cv::Point a, cv::Point b)
{
	return sqrt((double)((a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y)));
}

void cacher()
{
	cvNamedWindow("C", CV_WINDOW_AUTOSIZE );
	IplImage * imag = cvCreateImage(cvSize(WIDTH*4/DIV, HEIGHT*2),IPL_DEPTH_8U,3);
	cvZero(imag);
	cvShowImage("C", imag);
	cvMoveWindow("C", 1680+WIDTH*4/DIV-20, -30);

}//1680 1050

//initialise la classe (les deux écrans sont initialisés à la couleur noire, conformément à la chorégraphie
cultWindow::cultWindow(bool cache, Point _cursor)
	: image(NULL), pixels(NULL), sign(NULL), cursor(_cursor), cursorColor(CV_RGB(255, 255, 255)), color(0), currentStyle(Styles::NONEST), currentTool(Tools::NONET), tickness(0), busy(false), allowDraw(false), calType(Cals::NONCAL), calibrating(false), calProgress(0)
{
	if(cache)
		cacher();
	else
	{
		loadCal();

		cvNamedWindow("CULT", CV_WINDOW_AUTOSIZE );
		//main = cvCreateImage(cvSize(WIDTH*4/DIV, HEIGHT*2),IPL_DEPTH_8U,3);
		//cvZero(main);
		cvMoveWindow("CULT", 1670, -30);
		image = cvCreateImage(cvSize(WIDTH*4/DIV, HEIGHT*2),IPL_DEPTH_8U,3);
		cvZero(image);

	

		idxColorRock = 0;
		idxColorHipHop = 0;
		idxColorElectro = 0;

		pencilCount = 1;
		brushCount = 1;
		rockCount = 1;
		electroCount = 1;
		hiphopCount = 1;
		saveCount = 1;
		colorCount = 1;

		colorRock.push_back(CV_RGB(255, 237, 83));
		colorRock.push_back(CV_RGB(192, 0, 0));
		colorRock.push_back(CV_RGB(13, 13, 12));
		colorRock.push_back(CV_RGB(255, 255, 222));
		colorRock.push_back(CV_RGB(247, 17, 17));

		colorElectro.push_back(CV_RGB(169, 210, 240));
		colorElectro.push_back(CV_RGB(38, 202, 247));
		colorElectro.push_back(CV_RGB(232, 255, 12));
		colorElectro.push_back(CV_RGB(40, 255, 222));
		colorElectro.push_back(CV_RGB(247, 17, 212));

		colorHipHop.push_back(CV_RGB(0, 195, 209));
		colorHipHop.push_back(CV_RGB(53, 45, 10));
		colorHipHop.push_back(CV_RGB(235, 70, 43));
		colorHipHop.push_back(CV_RGB(230, 230, 230));
		colorHipHop.push_back(CV_RGB(255, 220, 146));
		//if(image)
			/*sign = cvLoadImage("..\\styles\\electro-01.jpg", 1);
			cvSetImageROI(image, cvRect(WIDTH*2/DIV - sign->width/2, HEIGHT - sign->height/2, sign->width, sign->height));
			cvCopy(pix, image, NULL);
			cvResetImageROI(image);*/

		//busy = true;
		//cvSet(image,cvScalarAll(0),NULL);
		pixels = cvCreateImage(cvSize(WIDTH*4/DIV, HEIGHT*2),IPL_DEPTH_8U,3);
		cvSet(pixels,cvScalarAll(0),NULL);

		cvShowImage("CULT", image);
	}
}

Point cultWindow::getCursor()
{
	return cursor;
}

//cette fonction permet de modifier la couleur actuelle en parcourant le cerle chromatique RGB du curseur (sera par la suite modifiée)
void cultWindow::incrementColor()
{
	string colorStr = "";
	if(currentStyle == Styles::ROCK)
	{
		cursorColor = colorRock[++idxColorRock%colorRock.size()];
		if(idxColorRock >= colorRock.size())
			idxColorRock = 0;
		if(idxColorRock == 1)
			colorStr = "Rouge";
		else if(idxColorRock == 2)
			colorStr = "Noir";
		else if(idxColorRock == 3)
			colorStr = "Gris clair";
		else if(idxColorRock == 4)
			colorStr = "Rouge clair";
		else if(idxColorRock == 0)
			colorStr = "Beige";
	}
	else if(currentStyle == Styles::HIPHOP)
	{
		cursorColor = colorHipHop[++idxColorHipHop%colorHipHop.size()];
		if(idxColorHipHop >= colorHipHop.size())
			idxColorHipHop = 0;
		if(idxColorHipHop == 0)
			colorStr = "Cyan";
		else if(idxColorHipHop == 1)
			colorStr = "Brun";
		else if(idxColorHipHop == 2)
			colorStr = "Rouge clair";
		else if(idxColorHipHop == 3)
			colorStr = "Gris clair";
		else if(idxColorHipHop == 4)
			colorStr = "Beige";
	}
	else if(currentStyle == Styles::ELECTRO)
	{
		cursorColor = colorElectro[++idxColorElectro%colorElectro.size()];
		if(idxColorElectro >= colorElectro.size())
			idxColorElectro = 0;
		if(idxColorElectro == 0)
			colorStr = "Cyan (1)";
		else if(idxColorElectro == 1)
			colorStr = "Cyan 2 (2)";
		else if(idxColorElectro == 2)
			colorStr = "Jaune (3)";
		else if(idxColorElectro == 3)
			colorStr = "Cyan 3 (4)";
		else if(idxColorElectro == 4)
			colorStr = "Rose (5)";
	}

	cout << "NEW COLOR : " << colorStr << endl;
	//770
	/*color += 20;
	if(color > 770)
		color = -770;

	int c1 = abs((int)color%255);
	int c2 = abs(255 - (int)color%255);

	if(color < -510)
		cursorColor = Scalar(c2, 0, c1);
	else if(color < -256)
		cursorColor = Scalar(0, c2, c1);
	else if(color < 0)
		cursorColor = Scalar(c1, c2, c1);
	else if(color < 256)
		cursorColor = Scalar(c2, c1, 0);
	else if(color < 510)
		cursorColor = Scalar(c1, 0, c2);
	else
		cursorColor = Scalar(c2, c1, c2);*/

	//cout << "RGB (" << color << "): " << cursorColor[0] << ";" << cursorColor[1] << ";" << cursorColor[2] << endl;
}

//permet de dessiner en tracant un cercle, puis en tracant un trait entre celui ci et le cercle n-1 si leur distance est inférieure à 100 (3 brushes différents pour les styles)
void cultWindow::draw(Point a, Point b)
{
	if(!busy)
	{
		if(currentTool != Tools::NONET && allowDraw)
		{
			//main = cvCreateImage(cvSize(WIDTH*4/DIV, HEIGHT*2),IPL_DEPTH_8U,3);
			if(currentStyle == Styles::ROCK)
			{
				/*cvCircle(pixels, a, tickness, cursorColor, -1, 8);
				cvCircle(pixels, b, tickness, cursorColor, -1, 8);*/

				Point cursorCornerLeftA(a.x-tickness/2, a.y+tickness/2), cursorCornerRightA(a.x+tickness/2, a.y-tickness/2);
				Point cursorCornerLeftB(b.x-tickness/2, b.y+tickness/2), cursorCornerRightB(b.x+tickness/2, b.y-tickness/2);
				cvRectangle(pixels, cursorCornerLeftA, cursorCornerRightA, cursorColor, -1);
				cvRectangle(pixels, cursorCornerLeftB, cursorCornerRightB, cursorColor, -1);

				//cout << cursorCornerLeftA.x << " " << cursorCornerLeftA.y << " " << cursorCornerRightA.x << " " << cursorCornerRightA.y << endl;

				if(getDist(a, b) < 100)
					if(currentTool == Tools::BRUSH)
						cvLine(pixels, a, b, cursorColor, (tickness), 0, 0);
					else
						cvLine(pixels, a, b, cursorColor, tickness, 0, 0);
			}
			else if(currentStyle == Styles::ELECTRO)
			{
				Scalar co1 = cursorColor;
				Scalar co2 = cursorColor;
	
				for(int j = 0 ; j < 3 ; j++)
				{
					co1[j] += 20;
					co2[j] += 40;
					if(co1[j] > 255)
						co1[j] = 255;
					if(co2[j] > 255)
						co2[j] = 255;
				}

				Point cursorCornerLeftA(a.x-tickness/2, a.y+tickness/2), cursorCornerRightA(a.x+tickness/2, a.y+tickness/2);
				Point cursorCornerLeftA2(a.x-tickness/2-5, a.y+tickness/2+5), cursorCornerRightA2(a.x+tickness/2+5, a.y-tickness/2-5);
				Point cursorCornerLeftA3(a.x-tickness/2-10, a.y+tickness/2+10), cursorCornerRightA3(a.x+tickness/2+10, a.y-tickness/2-10);
				Point cursorCornerLeftB(b.x-tickness/2, b.y+tickness/2), cursorCornerRightB(b.x+tickness/2, b.y-tickness/2);
				Point cursorCornerLeftB2(b.x-tickness/2-5, b.y+tickness/2+5), cursorCornerRightB2(b.x+tickness/2+5, b.y-tickness/2-5);
				Point cursorCornerLeftB3(b.x-tickness/2-10, b.y+tickness/2+10), cursorCornerRightB3(b.x+tickness/2+10, b.y-tickness/2-10);
			
				cvRectangle(pixels, cursorCornerLeftA, cursorCornerRightA, cursorColor, -1);
				cvRectangle(pixels, cursorCornerLeftA2, cursorCornerRightA2, co1, 5);
				cvRectangle(pixels, cursorCornerLeftA3, cursorCornerRightA3, co2, 10);
				cvRectangle(pixels, cursorCornerLeftB, cursorCornerRightB, cursorColor, -1);
				cvRectangle(pixels, cursorCornerLeftB2, cursorCornerRightB2, co1, 5);
				cvRectangle(pixels, cursorCornerLeftB3, cursorCornerRightB3, co2, 10);

				/*cvCircle(pixels, a, tickness+2, co2, -1, 8);
				cvCircle(pixels, b, tickness+2, co2, -1, 8);
				cvCircle(pixels, a, tickness+1, co1, -1, 8);
				cvCircle(pixels, b, tickness+1, co1, -1, 8);
				cvCircle(pixels, a, tickness, cursorColor, -1, 8);
				cvCircle(pixels, b, tickness, cursorColor, -1, 8);*/

				if(getDist(a, b) < 100)
					if(currentTool == Tools::BRUSH)
					{
						cvLine(pixels, a, b, co2, (tickness+35), 0);
						cvLine(pixels, a, b, co1, (tickness+30), 0);
						cvLine(pixels, a, b, cursorColor, (tickness+25), 0);
					}
					else
					{
						cvLine(pixels, a, b, co2, tickness+30, 8);
						cvLine(pixels, a, b, co1, tickness+25, 8);
						cvLine(pixels, a, b, cursorColor, tickness+20, 8);
					}
			}
			else if(currentStyle == Styles::HIPHOP)
			{
				Scalar co2 = cursorColor;
	
				/*for(int j = 0 ; j < 3 ; j++)
				{
					co2[j] += 20;
					if(co2[j] > 255)
						co2[j] = 255;
				}*/

				Point cursorCornerLeftA(a.x-tickness/2, a.y+tickness/2), cursorCornerRightA(a.x+tickness/2, a.y+tickness/2);
				Point cursorCornerLeftA2(a.x-tickness/2-10, a.y+tickness/2+10), cursorCornerRightA2(a.x+tickness/2+10, a.y-tickness/2-10);
				Point cursorCornerLeftB(b.x-tickness/2, b.y+tickness/2), cursorCornerRightB(b.x+tickness/2, b.y-tickness/2);
				Point cursorCornerLeftB2(b.x-tickness/2-10, b.y+tickness/2+10), cursorCornerRightB2(b.x+tickness/2+10, b.y-tickness/2-10);

				cvRectangle(pixels, cursorCornerLeftA, cursorCornerRightA, cursorColor, -1);
				//cvRectangle(pixels, cursorCornerLeftA2, cursorCornerRightA2, CV_RGB(155, 155, 155), 5);
				cvRectangle(pixels, cursorCornerLeftB, cursorCornerRightB, cursorColor, -1);
				//cvRectangle(pixels, cursorCornerLeftB2, cursorCornerRightB2, CV_RGB(155, 155, 155), 5);

				/*cvCircle(pixels, a, tickness+2, co2, -1, 8);
				cvCircle(pixels, b, tickness+2, co2, -1, 8);
				cvCircle(pixels, a, tickness+1, CV_RGB(0, 0, 0), -1, 8);
				cvCircle(pixels, b, tickness+1, CV_RGB(0, 0, 0), -1, 8);
				cvCircle(pixels, a, tickness, cursorColor, -1, 8);
				cvCircle(pixels, b, tickness, cursorColor, -1, 8);*/

				if(getDist(a, b) < 100)
					if(currentTool == Tools::BRUSH)
					{
						//cvLine(pixels, a, b, co2, (tickness+24), 8);
						cvLine(pixels, a, b, CV_RGB(155, 155, 155), (tickness+5), 8);
						cvLine(pixels, a, b, cursorColor, (tickness), 8);
					}
					else
					{
						//cvLine(pixels, a, b, co2, tickness+6, 8);
						cvLine(pixels, a, b, CV_RGB(155, 155, 155), tickness+4, 8);
						cvLine(pixels, a, b, cursorColor, tickness, 8);
					}
			}

			

			//cvCopy(pixels, main);
			//cvShowImage("CULT", main);

			//if(main)
				//cvReleaseImage(&main);

			//cout << "thickness : " << tickness << endl;
		}

		cvRectangle(pixels, Point(0, 0), Point(198, HEIGHT*2), CV_RGB(0, 0, 0), -1);
		cvRectangle(pixels, Point(660, 0), Point(WIDTH*4/DIV, HEIGHT*2), CV_RGB(0, 0, 0), -1);
		cvRectangle(pixels, Point(334, 0), Point(494, 260), CV_RGB(0, 0, 0), -1);
		cvRectangle(pixels, Point(0, 0), Point(WIDTH*4/DIV, 10), CV_RGB(0, 0, 0), -1);

		cvShowImage("CULT", pixels);
		/*
		194;492
		660;442
		334;160
		494;202
		*/
	}
}

//met à jour la position du curseur, mais l'affiche également dans l'écran virtuel (pas encore implémenté pour raisons de dégradation des performances)
void cultWindow::setCursor(Point _cursor)
{
	if(_cursor.x > 0 && _cursor.x < WIDTH*4/DIV && _cursor.y > 0 && _cursor.y < HEIGHT*2 && !busy)
	{
		cursor = _cursor;
		
		if(!allowDraw)
		{
			//main = cvCreateImage(cvSize(WIDTH*4/DIV, HEIGHT*2),IPL_DEPTH_8U,3);
			image = cvCreateImage(cvSize(WIDTH*4/DIV, HEIGHT*2),IPL_DEPTH_8U,3);
			cvZero(image);
			/*cvCircle(image, cursor, CURSORSIZE-2, Scalar(1.0f), 2);
			cvCircle(image, cursor, CURSORSIZE-4, cursorColor, -1);*/
			Point cursorCornerLeft(cursor.x-CURSORSIZE/3, cursor.y-CURSORSIZE/3), cursorCornerRight(cursor.x+CURSORSIZE/3, cursor.y+CURSORSIZE/3);
			Point cursorCornerBorderLeft(cursor.x-CURSORSIZE/2, cursor.y-CURSORSIZE/2), cursorCornerBorderRight(cursor.x+CURSORSIZE/2, cursor.y+CURSORSIZE/2);
			cvRectangle(image, cursorCornerBorderLeft, cursorCornerBorderRight, Scalar(1.0f), 2);
			cvRectangle(image, cursorCornerLeft, cursorCornerRight, cursorColor, -1);

			/*cvRectangle(image, Point(0, 0), Point(194, HEIGHT*2), CV_RGB(255, 255, 255), -1);
			cvRectangle(image, Point(660, 0), Point(660, HEIGHT*2), CV_RGB(255, 255, 255), -1);
			cvRectangle(image, Point(334, 0), Point(494, 200), CV_RGB(255, 255, 255), -1);*/

			cvShowImage("CULT", image);

			/*cvCopy(pixels, main);
			mask = cvCreateImage(cvGetSize(image), 8, 1);
			cvInRangeS(image, cvScalar(0.0, 0.0, 0.0), cvScalar(1.0, 1.0, 1.0), mask);
			cvNot(mask, mask);
			cvSetImageROI(image, cvRect(0, 0, image->width, image->height));
			cvCopy(image, main, mask);
			cvResetImageROI(image);
			cvShowImage("CULT", main);*/

			if(image)
				cvReleaseImage(&image);
			/*if(mask)
				cvReleaseImage(&mask);
			if(main)
				cvReleaseImage(&main);*/
			
		}
	}
}

double cultWindow::getDepth()
{
	return depth;
}

void cultWindow::setDepth(double _depth)
{
	depth = _depth;
}

cultWindow::Styles cultWindow::getStyle()
{
	return currentStyle;
}
void cultWindow::setStyle(cultWindow::Styles style)
{
	if(currentStyle == Styles::NONEST)
	{
		currentStyle = style;
		showStyle(style);
		cout << "STYLE CHOISI" << endl;
		//cout << "INST : " << style << endl;
	}
}

cultWindow::Tools cultWindow::getTools()
{
	return currentTool;
}
void cultWindow::setTools(cultWindow::Tools tool)
{
	if(currentStyle != Styles::NONEST && tool != currentTool)
	{
		currentTool = tool;
		if(currentTool == Tools::PENCIL)
			tickness = 5;
		else
			tickness = 20;
		//cout << "setTick(" << tool << ") : " << tickness << endl;
		showTool(tool);
		cout << "OUTIL CHOISI" << endl;
	}
}

//charge les images correspondantes aux styles, la met dans l'écran virtuel puis l'affiche pour indiquer la modification à l'utilisateur
void cultWindow::showStyle(cultWindow::Styles style)
{
	busy = true;

	if(!image)
		image = cvCreateImage(cvSize(WIDTH*4/DIV, HEIGHT*2),IPL_DEPTH_8U,3);

	cvZero(image);

	if(style == Styles::ELECTRO)
		sign = cvLoadImage("..\\styles\\electro-01.jpg", 1);
	else if(style == Styles::HIPHOP)
		sign = cvLoadImage("..\\styles\\hip_hop-01.jpg", 1);
	else
		sign = cvLoadImage("..\\styles\\rock-01.jpg", 1);

	cvSetImageROI(image, cvRect(WIDTH*2/DIV - sign->width/2, HEIGHT - sign->height/2, sign->width, sign->height));
	cvCopy(sign, image, NULL);
	cvResetImageROI(image);

	cvShowImage("CULT", image);
	this->Codes.clear();
	wait(2., true);
}

//idem pour les outils
void cultWindow::showTool(cultWindow::Tools tool)
{
	busy = true;
	allowDraw = false;

	if(!image)
		image = cvCreateImage(cvSize(WIDTH*4/DIV, HEIGHT*2),IPL_DEPTH_8U,3);
	cvZero(image);

	if(sign)
		cvReleaseImage(&sign);

	if(tool == Tools::PENCIL)
		sign = cvLoadImage("..\\tools\\pencil-01.jpg", 1);
	else
		sign = cvLoadImage("..\\tools\\brush-01.jpg", 1);
	
	cvSetImageROI(image, cvRect(WIDTH*2/DIV - sign->width/2, HEIGHT - sign->height/2, sign->width, sign->height));
	cvCopy(sign, image, NULL);
	cvResetImageROI(image);

	cvShowImage("CULT", image);
	this->Codes.clear();
	wait(2., true);
}

//idem pour le titre (actuellement utilisé lors d'une sauvegarde)
void cultWindow::showCult()
{
	busy = true;
	allowDraw = false;

	if(!image)
		image = cvCreateImage(cvSize(WIDTH*4/DIV, HEIGHT*2),IPL_DEPTH_8U,3);
	cvZero(image);

	if(sign)
		cvReleaseImage(&sign);

	sign = cvLoadImage("..\\cult.jpg", 1);
	
	cvSetImageROI(image, cvRect(WIDTH*2/DIV - sign->width/2, HEIGHT - sign->height/2, sign->width, sign->height));
	cvCopy(sign, image, NULL);
	cvResetImageROI(image);

	cvShowImage("CULT", image);
	this->Codes.clear();
	wait(3., true);
}

bool cultWindow::noStyle()
{
	return (currentStyle == Styles::NONEST);
}

void cultWindow::setAllowDraw(bool allow)
{
	allowDraw = allow;
}

string cultWindow::findStyle(Styles style)
{
	if(style == Styles::ELECTRO)
		return "ELECTRO";
	else if(style == Styles::HIPHOP)
		return "HIPHOP";
	else if(style == Styles::ROCK)
		return "ROCK";
}

void cultWindow::save()
{
	string nomFichier;
	if(!calibrating)
	{
		cout << "-= FILE SAVED =-" << endl;
		time_t now;
		char *_date;
		time(&now);
		_date = ctime(&now);
		string date = _date;

		nomFichier = "saved\\" + findStyle(currentStyle) + "-" + date.substr(0, date.length()-1) + ".jpg";

		replace(nomFichier.begin(), nomFichier.end(), ' ', '_');
		replace(nomFichier.begin(), nomFichier.end(), ':', '+');
	}

	//réinitialisation de l'interface et des variables en vue de permettre une réutilisation immédiate du dispositif
	allowDraw = false;
	busy = true;
	wait(5., false);
	currentTool = Tools::NONET;
	currentStyle = Styles::NONEST;

	if(image)
		cvReleaseImage(&image);
	
	if(!calibrating)
	{
		cout << nomFichier << endl;
		cvSaveImage(const_cast<char*>(nomFichier.c_str()), pixels);
	}

	if(pixels)
		cvReleaseImage(&pixels);
	if(sign)
		cvReleaseImage(&sign);
	image = cvCreateImage(cvSize(WIDTH*4/DIV, HEIGHT*2),IPL_DEPTH_8U,3);
	cvZero(image);
	pixels = cvCreateImage(cvSize(WIDTH*4/DIV, HEIGHT*2),IPL_DEPTH_8U,3);
	cvZero(pixels);
}

void cultWindow::loadCal()
{
	string line;
	ifstream myfile("calibrage.txt");
	if(calFileOK())
	{
		int i;
		getline(myfile, line);
		calPencil.push_back(atof(line.c_str()));
		for(i = 0 ; i < 2 ; i++)
		{
			getline(myfile, line);
			calSave.push_back(atof(line.c_str()));
		}
		for(i = 0 ; i < 2 ; i++)
		{
			getline(myfile, line);
			calRock.push_back(atof(line.c_str()));
		}
		for(i = 0 ; i < 2 ; i++)
		{
			getline(myfile, line);
			calColor.push_back(atof(line.c_str()));
		}
		for(i = 0 ; i < 3 ; i++)
		{
			getline(myfile, line);
			calElectro.push_back(atof(line.c_str()));
		}
		for(i = 0 ; i < 3 ; i++)
		{
			getline(myfile, line);
			calHipHop.push_back(atof(line.c_str()));
		}
		myfile.close();

		sort(calPencil.begin(), calPencil.end());
		sort(calSave.begin(), calSave.end());
		sort(calRock.begin(), calRock.end());
		sort(calColor.begin(), calColor.end());
		sort(calElectro.begin(), calElectro.end());
		sort(calHipHop.begin(), calHipHop.end());

		cout << "Fichier de calibrage charge" << endl;
	
		/*cout << "CAL PENCIL" << endl;
		cout << calPencil[0] << endl;
		cout << "CAL SAVE" << endl;
		for(i = 0 ; i < 2 ; i++)
			cout << calSave[i] << endl;
		cout << "CAL ROCKL" << endl;
		for(i = 0 ; i < 2 ; i++)
			cout << calRock[i] << endl;
		cout << "CAL COLOR" << endl;
		for(i = 0 ; i < 2 ; i++)
			cout << calColor[i] << endl;
		cout << "CAL ELECTRO" << endl;
		for(i = 0 ; i < 3 ; i++)
			cout << calElectro[i] << endl;
		cout << "CAL HIP HOP" << endl;
		for(i = 0 ; i < 3 ; i++)
			cout << calHipHop[i] << endl;*/
	}
	else
		cout << "Fichier de calibrage non trouve" << endl;
}

bool cultWindow::calFileOK()
{
	string line;
	int num = 0;
	ifstream myfile("calibrage.txt");
	if(myfile.is_open())
	{
		while(myfile.good())
		{
			getline(myfile, line);
			num++;
		}
	}
	//cout << "num : " << num << endl;
	return num == 14;
}

void cultWindow::launchCal()
{
	int i;

	/*cout << "CAL PENCIL" << endl;
	cout << calPencil[0] << endl;
	cout << "CAL SAVE" << endl;
	for(i = 0 ; i < 2 ; i++)
		cout << calSave[i] << endl;
	cout << "CAL ROCKL" << endl;
	for(i = 0 ; i < 2 ; i++)
		cout << calRock[i] << endl;
	cout << "CAL COLOR" << endl;
	for(i = 0 ; i < 2 ; i++)
		cout << calColor[i] << endl;
	cout << "CAL ELECTRO" << endl;
	for(i = 0 ; i < 3 ; i++)
		cout << calElectro[i] << endl;
	cout << "CAL HIP HOP" << endl;
	for(i = 0 ; i < 3 ; i++)
		cout << calHipHop[i] << endl;*/

	ofstream myfile("calibrage.txt", ios::trunc);
	if(myfile.is_open())
	{
		myfile << calPencil[0] << endl;
		for(i = 0 ; i < 2 ; i++)
			myfile << calSave[i] << endl;
		for(i = 0 ; i < 2 ; i++)
			myfile << calRock[i] << endl;
		for(i = 0 ; i < 2 ; i++)
			myfile << calColor[i] << endl;
		for(i = 0 ; i < 3 ; i++)
			myfile << calElectro[i] << endl;
		for(i = 0 ; i < 3 ; i++)
			myfile << calHipHop[i] << endl;
	}

	cout << "Fichier de calibrage cree" << endl;

	calibrating = false;
	calProgress = 0;
	calType = Cals::NONCAL;
	/*int calc = 0;
	double d1 = 0., d2 = 0., d3 = 0.;
	showTool(Tools::PENCIL);
	d1 += approxDist[size-3];
	d2 += approxDist[size-2];
	d3 += approxDist[size-1];
	
	if(calc > 100)
	{
		d1 /= calc;
		d2 /= calc;
		d3 /= calc;

		cout << d1 << endl;
		cout << d2 << endl;
		cout << d3 << endl;
		cin >> calc;
		if(calc < 100)
		{
			d1 = 0.;
			d2 = 0.;
			d3 = 0.;
		}
	}

	show*/
}