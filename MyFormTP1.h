#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <windows.h>
#include <msclr\marshal_cppstd.h>

#pragma unmanaged
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#pragma managed

namespace RandomNumberGenerator {
	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	using namespace msclr::interop;


	/// <summary>
	/// Summary for MyForm
	/// </summary>
	public ref class MyForm : public System::Windows::Forms::Form
	{
		//
		// Variables locales
		//
		cv::Mat* image0;
		cv::VideoCapture* cam = NULL;
		cv::VideoCapture* vid = NULL;
		char vsource = 'n';


		long calculerHistogramme(long freq[256][3], cv::Mat* image)
		{
			long max = -1;
			for (int i = 0; i < image->rows; i++)
				for (int j = 0; j < image->cols; j++)
				{
					cv::Vec3b& p = image->at<cv::Vec3b>(i, j);
					int B = p[0], G = p[1], R = p[2];
					freq[B][0]++;
					freq[G][1]++;
					freq[R][2]++;

					max = (max >= freq[B][0]) ? max : freq[B][0];
					max = (max >= freq[G][1]) ? max : freq[G][1];
					max = (max >= freq[R][2]) ? max : freq[R][2];
				}
			return max;
		}

		cv::Mat dessinerHistogramme(long freq[256][3], long max)
		{
			cv::Mat histo(200, 256, CV_8UC3, cv::Scalar(255, 255, 255));
			for (int i = 0; i < 256; i++)
			{
				if (i == 255) continue;

				int yB1 = 200 - freq[i][0] * 200.0 / max;
				int yG1 = 200 - freq[i][1] * 200.0 / max;
				int yR1 = 200 - freq[i][2] * 200.0 / max;
				int yB2 = 200 - freq[i + 1][0] * 200.0 / max;
				int yG2 = 200 - freq[i + 1][1] * 200.0 / max;
				int yR2 = 200 - freq[i + 1][2] * 200.0 / max;

				/* en courbes */
				line(histo, cv::Point(i, yB1), cv::Point(i + 1, yB2), cv::Scalar(255, 50, 20), 1, 8, 0);
				line(histo, cv::Point(i, yR1), cv::Point(i + 1, yR2), cv::Scalar(25, 50, 250), 1, 8, 0);
				line(histo, cv::Point(i, yG1), cv::Point(i + 1, yG2), cv::Scalar(40, 250, 20), 1, 8, 0);

				/* en barres
				if (lenG >= lenB && lenB >= lenR)
				{
					line(histo, cv::Point(i, 200), cv::Point(i, 200 - lenG), cv::Scalar(40, 250, 20), 2, 8, 0);
					line(histo, cv::Point(i, 200), cv::Point(i, 200 - lenB), cv::Scalar(255, 50, 20), 2, 8, 0);
					line(histo, cv::Point(i, 200), cv::Point(i, 200 - lenR), cv::Scalar(25, 50, 250), 2, 8, 0);
				}
				if (lenG >= lenR && lenR >= lenB)
				{
					line(histo, cv::Point(i, 200), cv::Point(i, 200 - lenG), cv::Scalar(40, 250, 20), 2, 8, 0);
					line(histo, cv::Point(i, 200), cv::Point(i, 200 - lenR), cv::Scalar(25, 50, 250), 2, 8, 0);
					line(histo, cv::Point(i, 200), cv::Point(i, 200 - lenB), cv::Scalar(255, 50, 20), 2, 8, 0);
				}
				if (lenB >= lenR && lenR >= lenG)
				{
					line(histo, cv::Point(i, 200), cv::Point(i, 200 - lenB), cv::Scalar(255, 50, 20), 2, 8, 0);
					line(histo, cv::Point(i, 200), cv::Point(i, 200 - lenR), cv::Scalar(25, 50, 250), 2, 8, 0);
					line(histo, cv::Point(i, 200), cv::Point(i, 200 - lenG), cv::Scalar(40, 250, 20), 2, 8, 0);
				}
				if (lenB >= lenG && lenG >= lenR)
				{
					line(histo, cv::Point(i, 200), cv::Point(i, 200 - lenB), cv::Scalar(255, 50, 20), 2, 8, 0);
					line(histo, cv::Point(i, 200), cv::Point(i, 200 - lenG), cv::Scalar(40, 250, 20), 2, 8, 0);
					line(histo, cv::Point(i, 200), cv::Point(i, 200 - lenR), cv::Scalar(25, 50, 250), 2, 8, 0);
				}
				if (lenR >= lenG && lenG >= lenB)
				{
					line(histo, cv::Point(i, 200), cv::Point(i, 200 - lenR), cv::Scalar(25, 50, 250), 2, 8, 0);
					line(histo, cv::Point(i, 200), cv::Point(i, 200 - lenG), cv::Scalar(40, 250, 20), 2, 8, 0);
					line(histo, cv::Point(i, 200), cv::Point(i, 200 - lenB), cv::Scalar(255, 50, 20), 2, 8, 0);
				}
				if (lenR >= lenB && lenB >= lenG)
				{
					line(histo, cv::Point(i, 200), cv::Point(i, 200 - lenR), cv::Scalar(25, 50, 250), 2, 8, 0);
					line(histo, cv::Point(i, 200), cv::Point(i, 200 - lenB), cv::Scalar(255, 50, 20), 2, 8, 0);
					line(histo, cv::Point(i, 200), cv::Point(i, 200 - lenG), cv::Scalar(40, 250, 20), 2, 8, 0);
				}
				*/

				/* Des points
				line(histo, cv::Point(i, 200 - lenB), cv::Point(i, 200 - lenB), cv::Scalar(255, 50, 20), 2, 8, 0);
				line(histo, cv::Point(i, 200 - lenR), cv::Point(i, 200 - lenR), cv::Scalar(25, 50, 250), 2, 8, 0);
				line(histo, cv::Point(i, 200 - lenG), cv::Point(i, 200 - lenG), cv::Scalar(40, 250, 20), 2, 8, 0);
				*/
			}

			return histo;
		}

		//
		// Fonctions Utilité
		//
		void activerBoutons(bool activer)
		{
			button1->Enabled = activer;
			button2->Enabled = activer;
			button3->Enabled = activer;
			button4->Enabled = activer;
			button6->Enabled = activer;
			button7->Enabled = activer;
			button8->Enabled = activer;
			button9->Enabled = activer;
			button10->Enabled = activer;
			button11->Enabled = activer;
			button15->Enabled = activer;
			button20->Enabled = activer;
		}

		std::string chooseAPictureDialog()
		{
			openFileDialog1->Filter = "Image PNG (*.png)|*.png|Image JPEG (*.jpg)|*.jpg|Image Bitmap (*.bmp)|*.bmp";

			if (openFileDialog1->ShowDialog() == System::Windows::Forms::DialogResult::OK)
				return marshal_as<std::string>(openFileDialog1->FileName);
			
			return "";
		}

		bool loadMatIntoPictureBox(cv::Mat& image, int pos)
		{
			try
			{
				System::Windows::Forms::Control^ pictureBox;
				if (pos % 4 == 1) pictureBox = pictureBox1;
				if (pos % 4 == 2) pictureBox = pictureBox2;
				if (pos % 4 == 3) pictureBox = pictureBox3;
				if (pos % 4 == 0) pictureBox = pictureBox4;

				Drawing::Graphics^ graphics = pictureBox->CreateGraphics();
				IntPtr ptr(image.ptr());
				Drawing::Bitmap^ b;
				if (pos < 5)
					b = gcnew Drawing::Bitmap(image.cols, image.rows, image.step, Drawing::Imaging::PixelFormat::Format24bppRgb, ptr);
				else
					b = gcnew Drawing::Bitmap(image.cols, image.rows, image.step, Drawing::Imaging::PixelFormat::Format8bppIndexed, ptr);
				Drawing::RectangleF rect(0, 0, pictureBox->Width, pictureBox->Height);
				graphics->DrawImage(b, rect);
				return true;
			}
			catch (System::Exception^ exep)
			{
				System::Windows::Forms::MessageBox::Show("Erreur : " + exep->Message);
				return false;
			}
		}

		bool loadImageIntoPictureBox(std::string location,  int pos)
		{
			if (location == "") return false;
			try
			{
				cv::Mat img = cv::imread(location, CV_LOAD_IMAGE_COLOR);
				image0 = new cv::Mat(img);
				return loadMatIntoPictureBox(img, 1);
			}
			catch (System::Exception^ exep)
			{
				System::Windows::Forms::MessageBox::Show(exep->Message);
				return false;
			}
		}

		cv::Mat getCanal(cv::Mat* image, int canal)
		{
			cv::Mat img = image->clone();

			for (int i = 0; i < img.rows; i++)
				for (int j = 0; j < img.cols; j++)
					for (int k = 0; k < 3; k++)
						if (k != canal) img.at<cv::Vec3b>(i, j)[k] = 0;

			return img;
		}

		//
		//	Gestion des événements
		//

		// Choisir une image
		private: System::Void button6_Click(System::Object^  sender, System::EventArgs^  e) {
			std::string filename = chooseAPictureDialog();
			loadImageIntoPictureBox(filename, 0);
			activerBoutons(true);
		}

		// Canal vert
		private: System::Void button8_Click(System::Object^  sender, System::EventArgs^  e) {
			cv::Mat CanalV = getCanal(image0, 1);
			loadMatIntoPictureBox(CanalV, 2);
		}

		// Canal Rouge
		private: System::Void button7_Click(System::Object^  sender, System::EventArgs^  e) {
			cv::Mat CanalV = getCanal(image0, 2);
			loadMatIntoPictureBox(CanalV, 3);
		}

		// Canal Bleu
		private: System::Void button9_Click(System::Object^  sender, System::EventArgs^  e) {
			cv::Mat CanalV = getCanal(image0, 0);
			loadMatIntoPictureBox(CanalV, 2);
		}

		// Grayscale
		private: System::Void button4_Click(System::Object^  sender, System::EventArgs^  e) {
			cv::Mat resultat;
			cv::cvtColor(*image0, resultat, CV_BGR2GRAY);

			for (int i = 0; i < resultat.rows; i++)
				for (int j = 0; j < resultat.cols; j++)
				{
					float B = image0->at<cv::Vec3b>(i, j)[0];
					float G = image0->at<cv::Vec3b>(i, j)[1];
					float R = image0->at<cv::Vec3b>(i, j)[2];
					resultat.at<uchar>(i, j) = 0.3*R + 0.59*G + 0.11*B;
				}

			cv::cvtColor(resultat, resultat, CV_GRAY2BGR);
			loadMatIntoPictureBox(resultat, 2);
		}

		// Seuillage / Threshold
		private: System::Void button1_Click_1(System::Object^  sender, System::EventArgs^  e) {
			cv::Mat resultat;
			cv::cvtColor(*image0, resultat, CV_BGR2GRAY);
			try
			{
				float thresh = (float)numTresh->Value;
				float max = (float)numMax->Value;

				int type;
				if (comboBox1->Text == "BINARY") type = cv::THRESH_BINARY;
				if (comboBox1->Text == "BINARY_INV") type = cv::THRESH_BINARY_INV;
				if (comboBox1->Text == "TRUNC") type = cv::THRESH_TRUNC;
				if (comboBox1->Text == "TOZERO") type = cv::THRESH_TOZERO;
				if (comboBox1->Text == "TOZERO_INV") type = cv::THRESH_TOZERO_INV;
				cv::threshold(resultat, resultat, thresh, max, type);

				cv::cvtColor(resultat, resultat, CV_GRAY2BGR);
				loadMatIntoPictureBox(resultat, 2);
			}
			catch (Exception^ Ex)
			{
				MessageBox::Show(Ex->Message + "\nErreur : Veuillez entrer un nombre valide");
				return;
			}
			
		}

		// Changer l'espace de couleurs
		private: System::Void button2_Click(System::Object^  sender, System::EventArgs^  e) {
			cv::Mat resultat; 
			int espace;

			if (comboColeurs->Text == "RGB") espace = CV_BGR2RGB;
			if (comboColeurs->Text == "HSV") espace = CV_BGR2HSV;
			if (comboColeurs->Text == "Lab") espace = CV_BGR2Lab;
			if (comboColeurs->Text == "GRAY") espace = CV_BGR2GRAY;

			cv::cvtColor(*image0, resultat, espace);
			if(espace == CV_BGR2GRAY) cv::cvtColor(resultat, resultat, CV_GRAY2BGR);
			
			loadMatIntoPictureBox(resultat, 2);

		}

		// Filtre Gaussien
		private: System::Void button3_Click(System::Object^  sender, System::EventArgs^  e) {
			try
			{
				float sigma = float::Parse(txtSigma->Text);
				int taille = (int)numericNGauss->Value;
				if (taille % 2 == 0) taille++;

				cv::Mat resultat;
				cv::GaussianBlur(*image0, resultat, cv::Size(taille, taille), sigma);
				loadMatIntoPictureBox(resultat, 3);
			}
			catch (Exception^ ex) 
			{
				MessageBox::Show(ex->Message + "\nVeuillez utiliser une virgule ',' pour les nombres réels.");
			}
		}

		// Filtre passe-bas  (Moyenneur ou Médian)
		private: System::Void button11_Click_1(System::Object^  sender, System::EventArgs^  e) {
			cv::Mat resultat;

			try 
			{
				int taille = (int)numericNMoy->Value;
				if (taille % 2 == 0) taille++;

				if (combofpb->Text == "Moyenneur")
				{
					cv::Mat fpb(taille, taille, CV_32F, cv::Scalar(1. / (taille*taille)));
					filter2D(*image0, resultat, -1, fpb, cv::Point(-1, -1), 0, cv::BORDER_DEFAULT);
				}
				if (combofpb->Text == "Median")
					cv::medianBlur(*image0, resultat, taille);

				loadMatIntoPictureBox(resultat, 3);
			}

			catch (Exception^ ex) 
			{
				MessageBox::Show(ex->Message); 
			}
		}

		// Filtre passe-haut
		private: System::Void button20_Click(System::Object^  sender, System::EventArgs^  e) {
			cv::Mat resultat;
			cv::Mat filtre;

			if (comboFPH->Text == "df/dx")
				filtre = (cv::Mat_<double>(1, 3) << 1, 0, -1);
			else if (comboFPH->Text == "df/dy")
				filtre = (cv::Mat_<double>(3, 1) << 1, 0, -1);
			else if (comboFPH->Text == "Laplacien v4")
				filtre = (cv::Mat_<double>(3, 3) << 0, 1, 0, 1, -4, 1, 0, 1, 0);
			else if (comboFPH->Text == "Laplacien v8")
				filtre = (cv::Mat_<double>(3, 3) << 1, 1, 1, 1, -8, 1, 1, 1, 1);
			else if (comboFPH->Text == "d2f/dx2")
				filtre = (cv::Mat_<double>(1, 3) << 1, -2, 1);
			else if (comboFPH->Text == "d2f/dy2")
				filtre = (cv::Mat_<double>(3, 1) << 1, -2, 1);
			else if (comboFPH->Text == "Sobel X")
				filtre = (cv::Mat_<double>(3, 3) << -1, 0, 1, -2, 0, 2, 1, 0, -1);
			else if (comboFPH->Text == "Sobel Y")
				filtre = (cv::Mat_<double>(3, 3) << -1, -2, -1, 0, 0, 0, 1, 2, 1);
			else if (comboFPH->Text == "Prewitt X")
				filtre = (cv::Mat_<double>(3, 3) << -1, 0, 1, -1, 0, 1, -1, 0, 1);
			else if (comboFPH->Text == "Prewitt Y")
				filtre = (cv::Mat_<double>(3, 3) << -1, -1, -1, 0, 0, 0, 1, 1, 1);
			else
			{
				MessageBox::Show("Ce filtre est invalide !");
				return;
			}
			filter2D(*image0, resultat, -1, filtre, cv::Point(-1, -1), 0, cv::BORDER_DEFAULT);
			loadMatIntoPictureBox(resultat, 3);
		}

		// Filtre Canny
		private: System::Void button15_Click_1(System::Object^  sender, System::EventArgs^  e) {
			try
			{
				cv::Mat resultat;
				int seuil = (int)numericUpDownSeuilCanny->Value;
				int r = (int)numericUpDownR->Value;
				float sigma = float::Parse(txtSigma->Text);
				int taille = (int)numericNGauss->Value;
				if (taille % 2 == 0) taille++;

				cvtColor(*image0, resultat, CV_BGR2GRAY);
				cv::GaussianBlur(resultat, resultat, cv::Size(taille, taille), sigma, sigma);
				cv::Canny(resultat, resultat, 0, seuil, r);
				cv::cvtColor(resultat, resultat, CV_GRAY2BGR);
				loadMatIntoPictureBox(resultat, 3);
			}
			catch (Exception^ ex)
			{
				MessageBox::Show(ex->Message + "\nVeuillez utiliser une virgule ',' pour les nombres réels.");
			}
		}


		// Afficher l'histogramme
		private: System::Void button6_Click_1(System::Object^  sender, System::EventArgs^  e) {
			long freq[256][3] = {0};
			long max = calculerHistogramme(freq, image0);
			cv::Mat histo = dessinerHistogramme(freq, max);

			loadMatIntoPictureBox(histo, 2);
		}
		
		// Egalisation d'histogramme
		private: System::Void button10_Click(System::Object^  sender, System::EventArgs^  e) {
			long freq[256][3] = { 0 };
			long freqAcc[256][3] = { 0 };

			long max = calculerHistogramme(freq, image0);

			// Calcul de l'histogramme cumulé
			freqAcc[0][0] = freq[0][0];
			freqAcc[0][1] = freq[0][1];
			freqAcc[0][2] = freq[0][2];

			for (int i = 1; i < 256; i++)
			{
				freqAcc[i][0] = freqAcc[i-1][0] + freq[i][0];
				freqAcc[i][1] = freqAcc[i-1][1] + freq[i][1];
				freqAcc[i][2] = freqAcc[i-1][2] + freq[i][2];
			}

			// Egaliser l'image
			cv::Mat imEgal = image0->clone();
			long n = imEgal.rows * imEgal.cols;

			for (int i = 0; i < imEgal.rows; i++)
				for (int j = 0; j < imEgal.cols; j++)
				{
					cv::Vec3b& p = imEgal.at<cv::Vec3b>(i, j);
					int B = p[0];
					int G = p[1];
					int R = p[2];

					p[0] = (255.0 / n)*freqAcc[B][0];
					p[1] = (255.0 / n)*freqAcc[G][1];
					p[2] = (255.0 / n)*freqAcc[R][2];
				}

			long freq2[256][3] = { 0 };
			long max2 = calculerHistogramme(freq2, &imEgal);
			cv::Mat histo = dessinerHistogramme(freq2, max2);

			loadMatIntoPictureBox(imEgal, 3);
			loadMatIntoPictureBox(histo, 4);
		}

		// Choisir une vidéo
		private: System::Void button12_Click(System::Object^  sender, System::EventArgs^  e) {
			std::string videopath = "";
			openFileDialog2->Filter = "Video MP4 (*.mp4)|*.mp4|Video AVI (*.avi)|*.avi";
			if (openFileDialog2->ShowDialog() != System::Windows::Forms::DialogResult::OK)
				return;
			videopath = marshal_as<std::string>(openFileDialog2->FileName);
			cv::VideoCapture capture(videopath);
			if (!capture.isOpened())
			{
				MessageBox::Show("Erreur : impossible de charger cette vidéo.");
				return;
			}

			vid = new cv::VideoCapture(videopath);
			timer1->Enabled = true;
			timer2->Enabled = false;
			vsource = 'v';
		}

		// Timer de	la video
		private: System::Void timer1_Tick_1(System::Object^  sender, System::EventArgs^  e) {
			if (vid == NULL)
				return;

			if (!(*vid).isOpened())  // La caméra ne marche pas
			{
				MessageBox::Show("Can't open the video");
				return;
			}
			cv::Mat frame;
			(*vid) >> frame;

			cv::Mat edges;

			cvtColor(frame, edges, CV_BGR2GRAY);
			cv::GaussianBlur(edges, edges, cv::Size(7, 7), 1.5, 1.5);
			cv::Canny(edges, edges, 0, 30, 3);
			cvtColor(edges, edges, CV_GRAY2BGR);

			loadMatIntoPictureBox(frame, 3);
			loadMatIntoPictureBox(edges, 4);
		}

		// Activer la Camera
		private: System::Void button14_Click(System::Object^  sender, System::EventArgs^  e) {
			timer2->Enabled = !timer2->Enabled;
			timer1->Enabled = false;
			vsource = 'c';
		}

		// Prendre un frame
		private: System::Void button13_Click(System::Object^  sender, System::EventArgs^  e) {
			if (image0) delete image0;

			cv::Mat frame;
			if (vsource == 'v') (*vid) >> frame;
			else if (vsource == 'c') (*cam) >> frame;
			else { MessageBox::Show("Veuillez activer une source de vidéo"); return; }
			image0 = new cv::Mat(frame);
			loadMatIntoPictureBox(frame, 1); 
			activerBoutons(true);
		}

		// Pause
		private: System::Void button5_Click_1(System::Object^  sender, System::EventArgs^  e) {
			if (vsource == 'v') timer1->Enabled = !timer1->Enabled;
			else if (vsource == 'c') timer2->Enabled = !timer2->Enabled;

			if (button5->Text == "Pause") button5->Text == "Reprendre";
			else button5->Text == "Pause";
		}

		// Préparer les timers pour les flux vidéo
		private: System::Void MyForm_Load(System::Object^  sender, System::EventArgs^  e) {
			int framerate = 33;  // intervale pour rafraichir l'image de la vidéo/caméra

			timer1->Interval = framerate;
			timer1->Enabled = false;
			timer2->Interval = framerate;
			timer2->Enabled = false;

			activerBoutons(false);
		}

		// Gérer le flux vidéo
		private: System::Void timer1_Tick(System::Object^  sender, System::EventArgs^  e) {
			if (cam == NULL)
				cam = new cv::VideoCapture(0);

			if (!(*cam).isOpened())  // si la caméra n'est pas fonctionnelle
			{
				MessageBox::Show("Erreur : impossible d'ouvrir la webcam.");
				return;
			}

			cv::Mat frame;
			(*cam) >> frame;

			cv::Mat edges;
			cvtColor(frame, edges, CV_BGR2GRAY);
			cv::GaussianBlur(edges, edges, cv::Size(7, 7), 1.5, 1.5);
			cv::Canny(edges, edges, 0, 30, 3);

			loadMatIntoPictureBox(frame, 3);
			loadMatIntoPictureBox(edges, 8);
		}

		// Gérer le flux Webcam
		private: System::Void timer2_Tick(System::Object^  sender, System::EventArgs^  e) {
			if (cam == NULL)
				cam = new cv::VideoCapture(0);

			if (!(*cam).isOpened())  // La caméra ne marche pas
			{
				MessageBox::Show("Can't open the cam !");
				return;
			}
			cv::Mat frame;
			(*cam) >> frame;

			cv::Mat edges;
			cv::cvtColor(frame, edges, CV_BGR2GRAY);
			cv::GaussianBlur(edges, edges, cv::Size(7, 7), 1.5, 1.5);
			cv::Canny(edges, edges, 0, 30, 3);
			cvtColor(edges, edges, CV_GRAY2BGR);

			loadMatIntoPictureBox(frame, 3);
			loadMatIntoPictureBox(edges, 4);
		}

		// Libérer les ressources lors de la fermeture
		private: System::Void MyForm_FormClosing(System::Object^  sender, System::Windows::Forms::FormClosingEventArgs^  e) {
			if (cam) cam->release();
			if (vid) vid->release();
		}
		public:
		MyForm(void)
		{
			InitializeComponent();
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~MyForm()
		{
			if (components)
			{
				delete components;
			}
		}

	private: System::Windows::Forms::PictureBox^  pictureBox1;
	private: System::Windows::Forms::PictureBox^  pictureBox2;
	private: System::Windows::Forms::PictureBox^  pictureBox3;
	private: System::Windows::Forms::PictureBox^  pictureBox4;
	private: System::Windows::Forms::OpenFileDialog^  openFileDialog1;
	private: System::Windows::Forms::Button^  buttonLoad;
	private: System::Windows::Forms::GroupBox^  groupBox1;
	private: System::Windows::Forms::Button^  button8;
	private: System::Windows::Forms::Button^  button4;
	private: System::Windows::Forms::Button^  button1;
	private: System::Windows::Forms::Label^  label2;
	private: System::Windows::Forms::Label^  label1;
	private: System::Windows::Forms::ComboBox^  comboBox1;
	private: System::Windows::Forms::NumericUpDown^  numMax;
	private: System::Windows::Forms::NumericUpDown^  numTresh;
	private: System::Windows::Forms::Button^  button2;
	private: System::Windows::Forms::ComboBox^  comboColeurs;
	private: System::Windows::Forms::Button^  button3;
	private: System::Windows::Forms::Button^  button6;
	private: System::Windows::Forms::Button^  button9;
	private: System::Windows::Forms::Button^  button7;
	private: System::Windows::Forms::Button^  button10;
	private: System::Windows::Forms::Timer^  timer1;
	private: System::Windows::Forms::Button^  button12;
	private: System::Windows::Forms::Timer^  timer2;
	private: System::Windows::Forms::Button^  button14;
	private: System::Windows::Forms::Button^  button13;
	private: System::Windows::Forms::Label^  label3;
	private: System::Windows::Forms::Label^  label4;
	private: System::Windows::Forms::NumericUpDown^  numericNGauss;
	private: System::Windows::Forms::Button^  button11;
	private: System::Windows::Forms::Label^  label5;
	private: System::Windows::Forms::NumericUpDown^  numericNMoy;
	private: System::Windows::Forms::TextBox^  txtSigma;
	private: System::Windows::Forms::OpenFileDialog^  openFileDialog2;
	private: System::Windows::Forms::ComboBox^  combofpb;
	private: System::Windows::Forms::Button^  button20;
	private: System::Windows::Forms::ComboBox^  comboFPH;
	private: System::Windows::Forms::Button^  button5;
	private: System::Windows::Forms::Label^  label7;
	private: System::Windows::Forms::NumericUpDown^  numericUpDownSeuilCanny;
	private: System::Windows::Forms::Label^  label6;
	private: System::Windows::Forms::NumericUpDown^  numericUpDownR;
	private: System::Windows::Forms::Button^  button15;
	private: System::Windows::Forms::Label^  label8;
	private: System::Windows::Forms::GroupBox^  groupBox2;
	private: System::Windows::Forms::GroupBox^  groupBox3;
	private: System::ComponentModel::IContainer^  components;

	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>

	#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>

		void InitializeComponent(void)
		{
			this->components = (gcnew System::ComponentModel::Container());
			this->pictureBox1 = (gcnew System::Windows::Forms::PictureBox());
			this->pictureBox2 = (gcnew System::Windows::Forms::PictureBox());
			this->pictureBox3 = (gcnew System::Windows::Forms::PictureBox());
			this->pictureBox4 = (gcnew System::Windows::Forms::PictureBox());
			this->openFileDialog1 = (gcnew System::Windows::Forms::OpenFileDialog());
			this->buttonLoad = (gcnew System::Windows::Forms::Button());
			this->groupBox1 = (gcnew System::Windows::Forms::GroupBox());
			this->label8 = (gcnew System::Windows::Forms::Label());
			this->button9 = (gcnew System::Windows::Forms::Button());
			this->button7 = (gcnew System::Windows::Forms::Button());
			this->button2 = (gcnew System::Windows::Forms::Button());
			this->comboColeurs = (gcnew System::Windows::Forms::ComboBox());
			this->numMax = (gcnew System::Windows::Forms::NumericUpDown());
			this->numTresh = (gcnew System::Windows::Forms::NumericUpDown());
			this->label2 = (gcnew System::Windows::Forms::Label());
			this->label1 = (gcnew System::Windows::Forms::Label());
			this->comboBox1 = (gcnew System::Windows::Forms::ComboBox());
			this->button1 = (gcnew System::Windows::Forms::Button());
			this->button4 = (gcnew System::Windows::Forms::Button());
			this->button8 = (gcnew System::Windows::Forms::Button());
			this->label7 = (gcnew System::Windows::Forms::Label());
			this->numericUpDownSeuilCanny = (gcnew System::Windows::Forms::NumericUpDown());
			this->label6 = (gcnew System::Windows::Forms::Label());
			this->numericUpDownR = (gcnew System::Windows::Forms::NumericUpDown());
			this->button15 = (gcnew System::Windows::Forms::Button());
			this->button5 = (gcnew System::Windows::Forms::Button());
			this->button20 = (gcnew System::Windows::Forms::Button());
			this->comboFPH = (gcnew System::Windows::Forms::ComboBox());
			this->combofpb = (gcnew System::Windows::Forms::ComboBox());
			this->txtSigma = (gcnew System::Windows::Forms::TextBox());
			this->label5 = (gcnew System::Windows::Forms::Label());
			this->numericNMoy = (gcnew System::Windows::Forms::NumericUpDown());
			this->button11 = (gcnew System::Windows::Forms::Button());
			this->label4 = (gcnew System::Windows::Forms::Label());
			this->numericNGauss = (gcnew System::Windows::Forms::NumericUpDown());
			this->label3 = (gcnew System::Windows::Forms::Label());
			this->button14 = (gcnew System::Windows::Forms::Button());
			this->button13 = (gcnew System::Windows::Forms::Button());
			this->button12 = (gcnew System::Windows::Forms::Button());
			this->button10 = (gcnew System::Windows::Forms::Button());
			this->button6 = (gcnew System::Windows::Forms::Button());
			this->button3 = (gcnew System::Windows::Forms::Button());
			this->timer1 = (gcnew System::Windows::Forms::Timer(this->components));
			this->timer2 = (gcnew System::Windows::Forms::Timer(this->components));
			this->openFileDialog2 = (gcnew System::Windows::Forms::OpenFileDialog());
			this->groupBox2 = (gcnew System::Windows::Forms::GroupBox());
			this->groupBox3 = (gcnew System::Windows::Forms::GroupBox());
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pictureBox1))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pictureBox2))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pictureBox3))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pictureBox4))->BeginInit();
			this->groupBox1->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numMax))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numTresh))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numericUpDownSeuilCanny))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numericUpDownR))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numericNMoy))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numericNGauss))->BeginInit();
			this->groupBox2->SuspendLayout();
			this->groupBox3->SuspendLayout();
			this->SuspendLayout();
			// 
			// pictureBox1
			// 
			this->pictureBox1->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
			this->pictureBox1->Location = System::Drawing::Point(18, 25);
			this->pictureBox1->Name = L"pictureBox1";
			this->pictureBox1->Size = System::Drawing::Size(256, 200);
			this->pictureBox1->TabIndex = 4;
			this->pictureBox1->TabStop = false;
			// 
			// pictureBox2
			// 
			this->pictureBox2->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
			this->pictureBox2->Location = System::Drawing::Point(18, 242);
			this->pictureBox2->Name = L"pictureBox2";
			this->pictureBox2->Size = System::Drawing::Size(256, 200);
			this->pictureBox2->SizeMode = System::Windows::Forms::PictureBoxSizeMode::StretchImage;
			this->pictureBox2->TabIndex = 9;
			this->pictureBox2->TabStop = false;
			// 
			// pictureBox3
			// 
			this->pictureBox3->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
			this->pictureBox3->Location = System::Drawing::Point(743, 25);
			this->pictureBox3->Name = L"pictureBox3";
			this->pictureBox3->Size = System::Drawing::Size(256, 200);
			this->pictureBox3->SizeMode = System::Windows::Forms::PictureBoxSizeMode::StretchImage;
			this->pictureBox3->TabIndex = 10;
			this->pictureBox3->TabStop = false;
			// 
			// pictureBox4
			// 
			this->pictureBox4->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
			this->pictureBox4->Location = System::Drawing::Point(743, 242);
			this->pictureBox4->Name = L"pictureBox4";
			this->pictureBox4->Size = System::Drawing::Size(256, 200);
			this->pictureBox4->SizeMode = System::Windows::Forms::PictureBoxSizeMode::StretchImage;
			this->pictureBox4->TabIndex = 11;
			this->pictureBox4->TabStop = false;
			// 
			// openFileDialog1
			// 
			this->openFileDialog1->FileName = L"openFileDialog1";
			// 
			// buttonLoad
			// 
			this->buttonLoad->Location = System::Drawing::Point(346, 20);
			this->buttonLoad->Name = L"buttonLoad";
			this->buttonLoad->Size = System::Drawing::Size(324, 38);
			this->buttonLoad->TabIndex = 12;
			this->buttonLoad->Text = L"Choisir une image";
			this->buttonLoad->UseVisualStyleBackColor = true;
			this->buttonLoad->Click += gcnew System::EventHandler(this, &MyForm::button6_Click);
			// 
			// groupBox1
			// 
			this->groupBox1->Controls->Add(this->label8);
			this->groupBox1->Controls->Add(this->button9);
			this->groupBox1->Controls->Add(this->button7);
			this->groupBox1->Controls->Add(this->button2);
			this->groupBox1->Controls->Add(this->comboColeurs);
			this->groupBox1->Controls->Add(this->numMax);
			this->groupBox1->Controls->Add(this->numTresh);
			this->groupBox1->Controls->Add(this->label2);
			this->groupBox1->Controls->Add(this->label1);
			this->groupBox1->Controls->Add(this->comboBox1);
			this->groupBox1->Controls->Add(this->button1);
			this->groupBox1->Controls->Add(this->button4);
			this->groupBox1->Controls->Add(this->button8);
			this->groupBox1->Location = System::Drawing::Point(296, 65);
			this->groupBox1->Name = L"groupBox1";
			this->groupBox1->Size = System::Drawing::Size(162, 378);
			this->groupBox1->TabIndex = 14;
			this->groupBox1->TabStop = false;
			this->groupBox1->Text = L"Transformations";
			// 
			// label8
			// 
			this->label8->AutoSize = true;
			this->label8->Location = System::Drawing::Point(15, 238);
			this->label8->Name = L"label8";
			this->label8->Size = System::Drawing::Size(40, 13);
			this->label8->TabIndex = 32;
			this->label8->Text = L"Type : ";
			// 
			// button9
			// 
			this->button9->Location = System::Drawing::Point(18, 90);
			this->button9->Name = L"button9";
			this->button9->Size = System::Drawing::Size(127, 28);
			this->button9->TabIndex = 31;
			this->button9->Text = L"Canal Bleu";
			this->button9->UseVisualStyleBackColor = true;
			this->button9->Click += gcnew System::EventHandler(this, &MyForm::button9_Click);
			// 
			// button7
			// 
			this->button7->Location = System::Drawing::Point(18, 56);
			this->button7->Name = L"button7";
			this->button7->Size = System::Drawing::Size(127, 28);
			this->button7->TabIndex = 30;
			this->button7->Text = L"Canal Rouge";
			this->button7->UseVisualStyleBackColor = true;
			this->button7->Click += gcnew System::EventHandler(this, &MyForm::button7_Click);
			// 
			// button2
			// 
			this->button2->Location = System::Drawing::Point(18, 338);
			this->button2->Name = L"button2";
			this->button2->Size = System::Drawing::Size(127, 28);
			this->button2->TabIndex = 26;
			this->button2->Text = L"Espace de Couleurs";
			this->button2->UseVisualStyleBackColor = true;
			this->button2->Click += gcnew System::EventHandler(this, &MyForm::button2_Click);
			// 
			// comboColeurs
			// 
			this->comboColeurs->FormattingEnabled = true;
			this->comboColeurs->Items->AddRange(gcnew cli::array< System::Object^  >(4) { L"RGB", L"HSV", L"GRAY", L"Lab" });
			this->comboColeurs->Location = System::Drawing::Point(18, 311);
			this->comboColeurs->Name = L"comboColeurs";
			this->comboColeurs->Size = System::Drawing::Size(127, 21);
			this->comboColeurs->TabIndex = 25;
			this->comboColeurs->Text = L"RGB";
			// 
			// numMax
			// 
			this->numMax->Location = System::Drawing::Point(81, 210);
			this->numMax->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 255, 0, 0, 0 });
			this->numMax->Name = L"numMax";
			this->numMax->Size = System::Drawing::Size(64, 20);
			this->numMax->TabIndex = 24;
			this->numMax->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 255, 0, 0, 0 });
			// 
			// numTresh
			// 
			this->numTresh->Location = System::Drawing::Point(81, 187);
			this->numTresh->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 255, 0, 0, 0 });
			this->numTresh->Name = L"numTresh";
			this->numTresh->Size = System::Drawing::Size(64, 20);
			this->numTresh->TabIndex = 23;
			this->numTresh->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 100, 0, 0, 0 });
			// 
			// label2
			// 
			this->label2->AutoSize = true;
			this->label2->Location = System::Drawing::Point(15, 212);
			this->label2->Name = L"label2";
			this->label2->Size = System::Drawing::Size(60, 13);
			this->label2->TabIndex = 22;
			this->label2->Text = L"MaxValue :";
			// 
			// label1
			// 
			this->label1->AutoSize = true;
			this->label1->Location = System::Drawing::Point(15, 189);
			this->label1->Name = L"label1";
			this->label1->Size = System::Drawing::Size(60, 13);
			this->label1->TabIndex = 20;
			this->label1->Text = L"Threshold :";
			// 
			// comboBox1
			// 
			this->comboBox1->FormattingEnabled = true;
			this->comboBox1->Items->AddRange(gcnew cli::array< System::Object^  >(5) {
				L"BINARY", L"BINARY_INV", L"TRUNC", L"TOZERO",
					L"TOZERO_INV"
			});
			this->comboBox1->Location = System::Drawing::Point(81, 235);
			this->comboBox1->Name = L"comboBox1";
			this->comboBox1->Size = System::Drawing::Size(64, 21);
			this->comboBox1->TabIndex = 19;
			this->comboBox1->Text = L"BINARY";
			// 
			// button1
			// 
			this->button1->Location = System::Drawing::Point(18, 261);
			this->button1->Name = L"button1";
			this->button1->Size = System::Drawing::Size(127, 28);
			this->button1->TabIndex = 16;
			this->button1->Text = L"Seuillage";
			this->button1->UseVisualStyleBackColor = true;
			this->button1->Click += gcnew System::EventHandler(this, &MyForm::button1_Click_1);
			// 
			// button4
			// 
			this->button4->Location = System::Drawing::Point(18, 136);
			this->button4->Name = L"button4";
			this->button4->Size = System::Drawing::Size(127, 28);
			this->button4->TabIndex = 15;
			this->button4->Text = L"Grayscale";
			this->button4->UseVisualStyleBackColor = true;
			this->button4->Click += gcnew System::EventHandler(this, &MyForm::button4_Click);
			// 
			// button8
			// 
			this->button8->Location = System::Drawing::Point(18, 22);
			this->button8->Name = L"button8";
			this->button8->Size = System::Drawing::Size(127, 28);
			this->button8->TabIndex = 14;
			this->button8->Text = L"Canal Vert";
			this->button8->UseVisualStyleBackColor = true;
			this->button8->Click += gcnew System::EventHandler(this, &MyForm::button8_Click);
			// 
			// label7
			// 
			this->label7->AutoSize = true;
			this->label7->Location = System::Drawing::Point(26, 126);
			this->label7->Name = L"label7";
			this->label7->Size = System::Drawing::Size(30, 13);
			this->label7->TabIndex = 59;
			this->label7->Text = L"Seuil";
			// 
			// numericUpDownSeuilCanny
			// 
			this->numericUpDownSeuilCanny->Location = System::Drawing::Point(58, 123);
			this->numericUpDownSeuilCanny->Name = L"numericUpDownSeuilCanny";
			this->numericUpDownSeuilCanny->Size = System::Drawing::Size(43, 20);
			this->numericUpDownSeuilCanny->TabIndex = 58;
			this->numericUpDownSeuilCanny->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 30, 0, 0, 0 });
			// 
			// label6
			// 
			this->label6->AutoSize = true;
			this->label6->Location = System::Drawing::Point(111, 125);
			this->label6->Name = L"label6";
			this->label6->Size = System::Drawing::Size(10, 13);
			this->label6->TabIndex = 57;
			this->label6->Text = L"r";
			// 
			// numericUpDownR
			// 
			this->numericUpDownR->Location = System::Drawing::Point(125, 123);
			this->numericUpDownR->Name = L"numericUpDownR";
			this->numericUpDownR->Size = System::Drawing::Size(30, 20);
			this->numericUpDownR->TabIndex = 56;
			this->numericUpDownR->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 3, 0, 0, 0 });
			// 
			// button15
			// 
			this->button15->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 8));
			this->button15->Location = System::Drawing::Point(164, 117);
			this->button15->Name = L"button15";
			this->button15->Size = System::Drawing::Size(61, 28);
			this->button15->TabIndex = 55;
			this->button15->Text = L"Canny";
			this->button15->UseVisualStyleBackColor = true;
			this->button15->Click += gcnew System::EventHandler(this, &MyForm::button15_Click_1);
			// 
			// button5
			// 
			this->button5->Location = System::Drawing::Point(22, 94);
			this->button5->Name = L"button5";
			this->button5->Size = System::Drawing::Size(82, 30);
			this->button5->TabIndex = 53;
			this->button5->Text = L"Pause";
			this->button5->UseVisualStyleBackColor = true;
			this->button5->Click += gcnew System::EventHandler(this, &MyForm::button5_Click_1);
			// 
			// button20
			// 
			this->button20->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 8));
			this->button20->Location = System::Drawing::Point(164, 83);
			this->button20->Name = L"button20";
			this->button20->Size = System::Drawing::Size(61, 28);
			this->button20->TabIndex = 52;
			this->button20->Text = L"FPH";
			this->button20->UseVisualStyleBackColor = true;
			this->button20->Click += gcnew System::EventHandler(this, &MyForm::button20_Click);
			// 
			// comboFPH
			// 
			this->comboFPH->FormattingEnabled = true;
			this->comboFPH->Items->AddRange(gcnew cli::array< System::Object^  >(8) {
				L"df/dx", L"df/dy", L"Laplacien v4", L"Laplacien v8",
					L"Sobel X", L"Sobel Y", L"Prewitt X", L"Prewitt Y"
			});
			this->comboFPH->Location = System::Drawing::Point(24, 88);
			this->comboFPH->Name = L"comboFPH";
			this->comboFPH->Size = System::Drawing::Size(131, 21);
			this->comboFPH->TabIndex = 51;
			this->comboFPH->Text = L"df/dx";
			// 
			// combofpb
			// 
			this->combofpb->FormattingEnabled = true;
			this->combofpb->Items->AddRange(gcnew cli::array< System::Object^  >(2) { L"Moyenneur", L"Median" });
			this->combofpb->Location = System::Drawing::Point(25, 51);
			this->combofpb->Name = L"combofpb";
			this->combofpb->Size = System::Drawing::Size(80, 21);
			this->combofpb->TabIndex = 50;
			this->combofpb->Text = L"Moyenneur";
			// 
			// txtSigma
			// 
			this->txtSigma->Location = System::Drawing::Point(61, 19);
			this->txtSigma->Name = L"txtSigma";
			this->txtSigma->Size = System::Drawing::Size(44, 20);
			this->txtSigma->TabIndex = 44;
			this->txtSigma->Text = L"1,5";
			// 
			// label5
			// 
			this->label5->AutoSize = true;
			this->label5->Location = System::Drawing::Point(109, 56);
			this->label5->Name = L"label5";
			this->label5->Size = System::Drawing::Size(15, 13);
			this->label5->TabIndex = 43;
			this->label5->Text = L"N";
			// 
			// numericNMoy
			// 
			this->numericNMoy->Location = System::Drawing::Point(126, 52);
			this->numericNMoy->Name = L"numericNMoy";
			this->numericNMoy->Size = System::Drawing::Size(30, 20);
			this->numericNMoy->TabIndex = 42;
			this->numericNMoy->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 5, 0, 0, 0 });
			// 
			// button11
			// 
			this->button11->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 8));
			this->button11->Location = System::Drawing::Point(164, 49);
			this->button11->Name = L"button11";
			this->button11->Size = System::Drawing::Size(63, 28);
			this->button11->TabIndex = 41;
			this->button11->Text = L"FPB";
			this->button11->UseVisualStyleBackColor = true;
			this->button11->Click += gcnew System::EventHandler(this, &MyForm::button11_Click_1);
			// 
			// label4
			// 
			this->label4->AutoSize = true;
			this->label4->Location = System::Drawing::Point(109, 22);
			this->label4->Name = L"label4";
			this->label4->Size = System::Drawing::Size(15, 13);
			this->label4->TabIndex = 40;
			this->label4->Text = L"N";
			// 
			// numericNGauss
			// 
			this->numericNGauss->Location = System::Drawing::Point(125, 19);
			this->numericNGauss->Name = L"numericNGauss";
			this->numericNGauss->Size = System::Drawing::Size(30, 20);
			this->numericNGauss->TabIndex = 39;
			this->numericNGauss->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 5, 0, 0, 0 });
			// 
			// label3
			// 
			this->label3->AutoSize = true;
			this->label3->Location = System::Drawing::Point(22, 23);
			this->label3->Name = L"label3";
			this->label3->Size = System::Drawing::Size(36, 13);
			this->label3->TabIndex = 38;
			this->label3->Text = L"Sigma";
			// 
			// button14
			// 
			this->button14->Location = System::Drawing::Point(22, 60);
			this->button14->Name = L"button14";
			this->button14->Size = System::Drawing::Size(202, 30);
			this->button14->TabIndex = 36;
			this->button14->Text = L"Activer la caméra";
			this->button14->UseVisualStyleBackColor = true;
			this->button14->Click += gcnew System::EventHandler(this, &MyForm::button14_Click);
			// 
			// button13
			// 
			this->button13->Location = System::Drawing::Point(111, 94);
			this->button13->Name = L"button13";
			this->button13->Size = System::Drawing::Size(113, 30);
			this->button13->TabIndex = 35;
			this->button13->Text = L"Prendre un frame";
			this->button13->UseVisualStyleBackColor = true;
			this->button13->Click += gcnew System::EventHandler(this, &MyForm::button13_Click);
			// 
			// button12
			// 
			this->button12->Location = System::Drawing::Point(22, 23);
			this->button12->Name = L"button12";
			this->button12->Size = System::Drawing::Size(202, 34);
			this->button12->TabIndex = 34;
			this->button12->Text = L"Choisir une video";
			this->button12->UseVisualStyleBackColor = true;
			this->button12->Click += gcnew System::EventHandler(this, &MyForm::button12_Click);
			// 
			// button10
			// 
			this->button10->Location = System::Drawing::Point(596, 251);
			this->button10->Name = L"button10";
			this->button10->Size = System::Drawing::Size(113, 34);
			this->button10->TabIndex = 32;
			this->button10->Text = L"Egalisation";
			this->button10->UseVisualStyleBackColor = true;
			this->button10->Click += gcnew System::EventHandler(this, &MyForm::button10_Click);
			// 
			// button6
			// 
			this->button6->Location = System::Drawing::Point(484, 251);
			this->button6->Name = L"button6";
			this->button6->Size = System::Drawing::Size(105, 34);
			this->button6->TabIndex = 29;
			this->button6->Text = L"Histogramme";
			this->button6->UseVisualStyleBackColor = true;
			this->button6->Click += gcnew System::EventHandler(this, &MyForm::button6_Click_1);
			// 
			// button3
			// 
			this->button3->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 8));
			this->button3->Location = System::Drawing::Point(164, 14);
			this->button3->Name = L"button3";
			this->button3->Size = System::Drawing::Size(63, 28);
			this->button3->TabIndex = 27;
			this->button3->Text = L"Gaussien";
			this->button3->UseVisualStyleBackColor = true;
			this->button3->Click += gcnew System::EventHandler(this, &MyForm::button3_Click);
			// 
			// timer1
			// 
			this->timer1->Tick += gcnew System::EventHandler(this, &MyForm::timer1_Tick_1);
			// 
			// timer2
			// 
			this->timer2->Tick += gcnew System::EventHandler(this, &MyForm::timer2_Tick);
			// 
			// openFileDialog2
			// 
			this->openFileDialog2->FileName = L"openFileDialog2";
			// 
			// groupBox2
			// 
			this->groupBox2->Controls->Add(this->button15);
			this->groupBox2->Controls->Add(this->label7);
			this->groupBox2->Controls->Add(this->label4);
			this->groupBox2->Controls->Add(this->numericNGauss);
			this->groupBox2->Controls->Add(this->numericUpDownSeuilCanny);
			this->groupBox2->Controls->Add(this->button11);
			this->groupBox2->Controls->Add(this->label3);
			this->groupBox2->Controls->Add(this->label6);
			this->groupBox2->Controls->Add(this->numericNMoy);
			this->groupBox2->Controls->Add(this->label5);
			this->groupBox2->Controls->Add(this->numericUpDownR);
			this->groupBox2->Controls->Add(this->txtSigma);
			this->groupBox2->Controls->Add(this->combofpb);
			this->groupBox2->Controls->Add(this->comboFPH);
			this->groupBox2->Controls->Add(this->button3);
			this->groupBox2->Controls->Add(this->button20);
			this->groupBox2->Location = System::Drawing::Point(470, 64);
			this->groupBox2->Name = L"groupBox2";
			this->groupBox2->Size = System::Drawing::Size(247, 161);
			this->groupBox2->TabIndex = 60;
			this->groupBox2->TabStop = false;
			this->groupBox2->Text = L"Filtres";
			// 
			// groupBox3
			// 
			this->groupBox3->Controls->Add(this->button12);
			this->groupBox3->Controls->Add(this->button14);
			this->groupBox3->Controls->Add(this->button13);
			this->groupBox3->Controls->Add(this->button5);
			this->groupBox3->Location = System::Drawing::Point(470, 299);
			this->groupBox3->Name = L"groupBox3";
			this->groupBox3->Size = System::Drawing::Size(247, 143);
			this->groupBox3->TabIndex = 61;
			this->groupBox3->TabStop = false;
			this->groupBox3->Text = L"Vidéo";
			// 
			// MyForm
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->BackColor = System::Drawing::SystemColors::ButtonHighlight;
			this->ClientSize = System::Drawing::Size(1020, 490);
			this->Controls->Add(this->groupBox3);
			this->Controls->Add(this->groupBox2);
			this->Controls->Add(this->groupBox1);
			this->Controls->Add(this->pictureBox4);
			this->Controls->Add(this->pictureBox3);
			this->Controls->Add(this->pictureBox2);
			this->Controls->Add(this->pictureBox1);
			this->Controls->Add(this->buttonLoad);
			this->Controls->Add(this->button6);
			this->Controls->Add(this->button10);
			this->Name = L"MyForm";
			this->Text = L"TP1 - Traitement d\'Images";
			this->FormClosing += gcnew System::Windows::Forms::FormClosingEventHandler(this, &MyForm::MyForm_FormClosing);
			this->Load += gcnew System::EventHandler(this, &MyForm::MyForm_Load);
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pictureBox1))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pictureBox2))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pictureBox3))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pictureBox4))->EndInit();
			this->groupBox1->ResumeLayout(false);
			this->groupBox1->PerformLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numMax))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numTresh))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numericUpDownSeuilCanny))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numericUpDownR))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numericNMoy))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numericNGauss))->EndInit();
			this->groupBox2->ResumeLayout(false);
			this->groupBox2->PerformLayout();
			this->groupBox3->ResumeLayout(false);
			this->ResumeLayout(false);

		}
	#pragma endregion
};
}