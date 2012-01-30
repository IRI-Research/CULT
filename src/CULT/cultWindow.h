#ifndef CULT
#define CULT

//�l�ments d�finissant l'interface et les options de chor�graphie (timer, pas de d�tection...)

/*#define WIDTH 640
#define HEIGHT 480*/

#define WIDTH 640
#define HEIGHT 480

#define DIV 3
#define CURSORSIZE 10

#define CONLEFT 205
#define CONRIGHT WIDTH*4/DIV-225
#define CONTOP 90
#define CONMIDTOP 290
#define CONMIDLEFT WIDTH*4/DIV
#define CONMIDRIGHT 250

#include <iostream>
#include <ctime>
#include <vector>
#include <fstream>
#include <cstdlib>

// opencv
#pragma comment(lib, "cxcore210d.lib")
#pragma comment(lib, "cv210d.lib")
#pragma comment(lib, "highgui210d.lib")
#include <opencv/highgui.h>
#include <opencv/cv.h>

class cultWindow
{
	public:
		//d�finition des styles et outils
		enum Styles {ROCK, ELECTRO, HIPHOP, NONEST};
		enum Tools {PENCIL, BRUSH, NONET};
		enum Cals {CPENCIL, CSAVE, CROCK, CCOLOR, CELECTRO, CHIPHOP, NONCAL};
		//ce vecteur empilera les codes correspondants aux gestures/outils d�tect�s au cours d'une "session" (deux intervalles de timer)
		std::vector<int> Codes;

		std::vector<cv::Scalar> colorHipHop;
		std::vector<cv::Scalar> colorRock;
		std::vector<cv::Scalar> colorElectro;

		int idxColorRock;
		int idxColorHipHop;
		int idxColorElectro;

		cultWindow(bool cache = false, cv::Point _cursor = cv::Point(0., 0.));
		//le curseur, repr�sent� dans l'�cran virtuel par une mire
		cv::Point getCursor();
		void setCursor(cv::Point _cursor);
		//fonctions principales de dessin sur l'�cran principal
		void draw(cv::Point a, cv::Point b);
		void incrementColor();
		//permet d'acc�der � la profondeur (pas encore impl�ment�)
		double getDepth();
		void setDepth(double _depth);
		//idem pour le style
		Styles getStyle();
		void setStyle(Styles style);
		//idem pour les outils
		Tools getTools();
		void setTools(Tools tool);
		//permet d'afficher les outils sur l'�cran virtuel et de montrer durant n sec l'�cran virtuel (de mani�re � afficher � l'utilisateur la validation de la d�tection)
		void showStyle(Styles style);
		void showTool(Tools tool);
		//true si encore aucun style d�tect�
		bool noStyle();
		//fonctions de timer
		void wait(double s, bool init);
		void waitFlush(double s, bool init);
		//fonction qui bloque/d�bloque la capacit� de l'�cran principal � dessiner
		void setAllowDraw(bool allow);
		//traduit un code de style en string (nom du style, pour la sauvegarde)
		std::string findStyle(Styles style);
		void save();
		//affiche l'�cran virtuel avec le nom du prototype
		void showCult();

		void loadCal();
		void launchCal();
		bool calFileOK();

		int pencilCount;
		int brushCount;
		int rockCount;
		int electroCount;
		int hiphopCount;
		int saveCount;
		int colorCount;

		std::vector<double> calPencil;
		std::vector<double> calSave;
		std::vector<double> calRock;
		std::vector<double> calBrush;
		std::vector<double> calColor;
		std::vector<double> calElectro;
		std::vector<double> calHipHop;

		Cals calType;
		int calProgress;

		bool calibrating;

		cv::Point cursor;

	private:
		//cv::Point cursor;
		Styles currentStyle;
		Tools currentTool;
		double depth;
		cv::Scalar cursorColor;
		double color;
		int tickness;
		bool busy;
		bool allowDraw;
		//�cran virtuel, �cran principal, �cran de charge d'images fusionn� � l'�cran virtuel de facon � afficher ponctuellement les �v�nements en cours (ex : style rock d�tect�)
		IplImage *image, *pixels, *sign, *main, *mask;
};

#endif