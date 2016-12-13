/*
 * File   : MyForm.h
 * Desc   : Contains the entire project (thus extremely messy).
 * Author : Ismail Harrando
 */

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

	typedef struct Pt
	{
		int i; int j; float val;
	} Pt;

	bool comparer(Pt p, Pt q) { return p.val > q.val; }
	bool comparer2(Pt p, Pt q) { return p.j > q.j; }

	/// <summary>
	/// Summary for MyForm
	/// </summary>
	public ref class MyForm : public System::Windows::Forms::Form
	{
		//
		// Variables locales
		//
		cv::Mat* image0;
		cv::Mat* imagef, imageg, imager, imagep;
		static int compteur;

		//
		// Fonctions Utilité
		//
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
				if (pos % 4 == 0)  pictureBox = pictureBox4;

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
				System::Windows::Forms::MessageBox::Show(exep->Message);
				return false;
			}
		}

		bool loadImageIntoPictureBox(std::string location, int pos)
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

		// Calculer la matrice de Harris
		cv::Mat calculerHarris(cv::Mat* image, bool filtrer, bool domination, float s, float k)
		{
			int taille, nmax;
			float sigma;

			try
			{
				nmax = (int)numericUpDownNMax->Value;
				taille = (int)numericNGauss->Value;

				if (taille % 2 == 0) taille++;
				sigma = float::Parse(txtSigma->Text);
			}
			catch (Exception^ ex)
			{
				MessageBox::Show(ex->Message + "\nVeuillez utiliser une virgule ',' pour les valeurs réelles de Sigma.");
				return *image;
			}

			if (image == NULL)
				return cv::Mat();
			imagep = new cv::Mat(image->clone());
			cv::Mat img = cv::Mat(image->clone());
			cv::Mat imgGris, Ix, Iy, Ixy, Ix2, Iy2;
			cv::Mat Det, Trace, H, H_norm, H_avant;

			cv::cvtColor(img, imgGris, CV_BGR2GRAY);

			// Calculer Ix, Iy, Ixy, Ix2, Iy2
			cv::Mat SobelX = (cv::Mat_<double>(3, 3) << -1, 0, 1, -2, 0, 2, -1, 0, 1);
			cv::Mat SobelY = (cv::Mat_<double>(3, 3) << -1, -2, -1, 0, 0, 0, 1, 2, 1);
			filter2D(imgGris, Ix, CV_32FC1, SobelX, cv::Point(-1, -1), 0, cv::BORDER_DEFAULT);
			filter2D(imgGris, Iy, CV_32FC1, SobelY, cv::Point(-1, -1), 0, cv::BORDER_DEFAULT);

			Ix2 = Ix.mul(Ix);
			Iy2 = Iy.mul(Iy);
			Ixy = Ix.mul(Iy);
			if (filtrer)
			{
				cv::GaussianBlur(Ix2, Ix2, cv::Size(taille, taille), sigma, 0);
				cv::GaussianBlur(Iy2, Iy2, cv::Size(taille, taille), 0, sigma);
				cv::GaussianBlur(Ixy, Ixy, cv::Size(taille, taille), sigma, sigma);
			}
			Det = Ix2.mul(Iy2) - Ixy.mul(Ixy);
			// Trace = Ix2 + Iy2;
			Trace = (Ix2 + Iy2).mul(Ix2 + Iy2);
			H = Det - k * Trace;

			/*
			double minH, maxH;
			cv::minMaxLoc(H, &minH, &maxH);
			MessageBox::Show("H  /  Max : " + maxH + "  /  Min : " + minH);
			*/

			// Normaliser les valeurs de H sur l'échelle [0,255]
			normalize(H, H_norm, 0, 255, cv::NORM_MINMAX, CV_32FC1, cv::Mat());

			// Afficher H avant le seuillage et l'élimination des non-maxima
			convertScaleAbs(H_norm, H_avant);
			cv::cvtColor(H_avant, H_avant, CV_GRAY2BGR);
			loadMatIntoPictureBox(H_avant, 2);

			long nbpoints = 0;
			std::vector<Pt> points;

			// Garder les pixels dominants
			for (int i = 0, imax = H_norm.rows - 1; i < H_norm.rows; i++)
				for (int j = 0, jmax = H_norm.cols - 1; j < H_norm.cols; j++)
				{
					float v = H_norm.at<float>(i, j);

					if (domination)
					{
						if (checkBox1->Checked && i > 0 && j > 0 && v <= H_norm.at<float>(i - 1, j - 1) ||
							checkBox2->Checked && j > 0 && v <= H_norm.at<float>(i, j - 1) ||
							checkBox3->Checked && i < imax && j > 0 && v <= H_norm.at<float>(i + 1, j - 1) ||
							checkBox4->Checked && i > 0 && v <= H_norm.at<float>(i - 1, j) ||
							checkBox5->Checked && i < imax && v <= H_norm.at<float>(i + 1, j) ||
							checkBox6->Checked && i > 0 && j < jmax && v <= H_norm.at<float>(i - 1, j + 1) ||
							checkBox7->Checked && j < jmax && v <= H_norm.at<float>(i, j + 1) ||
							checkBox8->Checked && i < imax && j < jmax && v <= H_norm.at<float>(i + 1, j + 1))

							H_norm.at<float>(i, j) = 0;
					}

					if (H_norm.at<float>(i, j) <= s)
						H_norm.at<float>(i, j) = 0;
				}

			// Mettre les points d'intérêt dans un vecteur
			for (int i = 0; i < H_norm.rows; i++)
				for (int j = 0; j < H_norm.cols; j++)
					if (H_norm.at<float>(i, j) > s)
						points.push_back(Pt{ j, i, H_norm.at<float>(i, j) });

			// Trier les points par leur valeur
			std::sort(points.begin(), points.end(), comparer);

			// Afficher des cercles autour des nmax premiers points d'intéret
			int c;
			for (c = 0; c < points.size() && c < nmax; c++)
				circle(*imagep, cv::Point(points[c].i, points[c].j), 7, cv::Scalar(20, 20, 245), 2, 8, 0);
			loadMatIntoPictureBox(*imagep, 3);
			labelAffiches->Text = "Points d'intérêt affichés : " + c;
			labelTrouves->Text = "Points d'intérêt trouvés : " + points.size();

			// Passer de float à char et d'une à trois canaux
			convertScaleAbs(H_norm, H_norm);
			cv::cvtColor(H_norm, H_norm, CV_GRAY2BGR);
			return H_norm;
		}

		// Harris avec renvoi de Points d'Intéret
		std::vector<Pt> calculerHarris2(cv::Mat* image, bool filtrer, bool domination, float s, float k)
		{
			int taille, nmax;
			float sigma;
			try
			{
				nmax = (int)numericUpDownNMax->Value;
				taille = (int)numericNGauss->Value;

				if (taille % 2 == 0) taille++;
				sigma = float::Parse(txtSigma->Text);
			}
			catch (Exception^ ex)
			{
				MessageBox::Show(ex->Message + "\nVeuillez utiliser une virgule ',' pour les valeurs réelles de Sigma.");
				return *image;
			}

			imagep = new cv::Mat(image->clone());
			cv::Mat img = cv::Mat(image->clone());
			cv::Mat imgGris, Ix, Iy, Ixy, Ix2, Iy2;
			cv::Mat Det, Trace, H, H_norm, H_avant;

			cv::cvtColor(img, imgGris, CV_BGR2GRAY);

			// Calculer Ix, Iy, Ixy, Ix2, Iy2
			cv::Mat SobelX = (cv::Mat_<double>(3, 3) << -1, 0, 1, -2, 0, 2, -1, 0, 1);
			cv::Mat SobelY = (cv::Mat_<double>(3, 3) << -1, -2, -1, 0, 0, 0, 1, 2, 1);
			filter2D(imgGris, Ix, CV_32FC1, SobelX, cv::Point(-1, -1), 0, cv::BORDER_DEFAULT);
			filter2D(imgGris, Iy, CV_32FC1, SobelY, cv::Point(-1, -1), 0, cv::BORDER_DEFAULT);

			Ix2 = Ix.mul(Ix);
			Iy2 = Iy.mul(Iy);
			Ixy = Ix.mul(Iy);
			if (filtrer)
			{
				cv::GaussianBlur(Ix2, Ix2, cv::Size(taille, taille), sigma, 0);
				cv::GaussianBlur(Iy2, Iy2, cv::Size(taille, taille), 0, sigma);
				cv::GaussianBlur(Ixy, Ixy, cv::Size(taille, taille), sigma, sigma);
			}
			Det = Ix2.mul(Iy2) - Ixy.mul(Ixy);
			Trace = (Ix2 + Iy2).mul(Ix2 + Iy2);
			H = Det - k * Trace;

			normalize(H, H_norm, 0, 255, cv::NORM_MINMAX, CV_32FC1, cv::Mat());

			// Afficher H avant le seuillage et l'élimination des non-maxima
			/* convertScaleAbs(H_norm, H_avant);
			cv::cvtColor(H_avant, H_avant, CV_GRAY2BGR);
			loadMatIntoPictureBox(H_avant, 2); */

			long nbpoints = 0;
			std::vector<Pt> points;

			// Garder les pixels dominants
			for (int i = 0, imax = H_norm.rows - 1; i < H_norm.rows; i++)
				for (int j = 0, jmax = H_norm.cols - 1; j < H_norm.cols; j++)
				{
					float v = H_norm.at<float>(i, j);

					if (domination)
					{
						if (checkBox1->Checked && i > 0 && j > 0 && v <= H_norm.at<float>(i - 1, j - 1) ||
							checkBox2->Checked && j > 0 && v <= H_norm.at<float>(i, j - 1) ||
							checkBox3->Checked && i < imax && j > 0 && v <= H_norm.at<float>(i + 1, j - 1) ||
							checkBox4->Checked && i > 0 && v <= H_norm.at<float>(i - 1, j) ||
							checkBox5->Checked && i < imax && v <= H_norm.at<float>(i + 1, j) ||
							checkBox6->Checked && i > 0 && j < jmax && v <= H_norm.at<float>(i - 1, j + 1) ||
							checkBox7->Checked && j < jmax && v <= H_norm.at<float>(i, j + 1) ||
							checkBox8->Checked && i < imax && j < jmax && v <= H_norm.at<float>(i + 1, j + 1))

							H_norm.at<float>(i, j) = 0;
					}

					if (H_norm.at<float>(i, j) <= s)
						H_norm.at<float>(i, j) = 0;
				}

			// Mettre les points d'intérêt dans un vecteur
			for (int i = 0; i < H_norm.rows; i++)
				for (int j = 0; j < H_norm.cols; j++)
					if (H_norm.at<float>(i, j) > s)
						points.push_back(Pt{ j, i, H_norm.at<float>(i, j) });

			// Trier les points par leur valeur
			std::sort(points.begin(), points.end(), comparer);

			// Afficher des cercles autour des nmax premiers points d'intéret
			int c;
			for (c = 0; c < points.size() && c < nmax; c++)
				circle(*imagep, cv::Point(points[c].i, points[c].j), 7, cv::Scalar(20, 20, 245), 2, 8, 0);

			// std::sort(points.begin(), points.end(), comparer2);
			return points;
		}


		// Ouvrir les images Gauche et Droite
		bool ouvrirImagesLR(cv::Mat* img1, cv::Mat* img2)
		{
			std::string filename1, filename2;
			openFileDialog3->Filter = "Image PNG (*.png)|*.png|Image JPEG (*.jpg)|*.jpg|Image Bitmap (*.bmp)|*.bmp";

			MessageBox::Show("Sélectionner l'image Gauche (L) : ");
			if (openFileDialog3->ShowDialog() == System::Windows::Forms::DialogResult::OK)
				filename1 = marshal_as<std::string>(openFileDialog3->FileName);
			else return false;

			MessageBox::Show("Sélectionner l'image Droite (R) : ");
			if (openFileDialog3->ShowDialog() == System::Windows::Forms::DialogResult::OK)
				filename2 = marshal_as<std::string>(openFileDialog3->FileName);
			else return false;

			if (filename1 == "" || filename2 == "") return false;
			try
			{
				*img1 = cv::imread(filename1, CV_LOAD_IMAGE_COLOR);
				*img2 = cv::imread(filename2, CV_LOAD_IMAGE_COLOR);

				if (img1->rows != img2->rows || img1->cols != img2->cols)
				{
					System::Windows::Forms::MessageBox::Show("Les deux images doivent être de la même taille.");
					return false;
				}

				loadMatIntoPictureBox(*img1, 1);
				loadMatIntoPictureBox(*img2, 3);
			}
			catch (System::Exception^ exep)
			{
				System::Windows::Forms::MessageBox::Show("Erreur : " + exep->Message);
				return false;
			}
			return true;
		}

		// Calculer une valeur moyenne d'un pixel 3D
		inline float val(cv::Vec3b& p) { return (p[0] + p[1] + p[2]) / 3; }

		//
		//	Gestion des événements
		//

		// Choisir une image
		private: System::Void button6_Click(System::Object^  sender, System::EventArgs^  e) {
			std::string filename = chooseAPictureDialog();
			loadImageIntoPictureBox(filename, 0);
		}

		// Filtre Gaussien avec paramètres
		private: System::Void button3_Click(System::Object^  sender, System::EventArgs^  e) {
			
			try
			{
				int taille = (int)numericNGauss->Value;
				if (taille % 2 == 0) taille++;
				float sigma = float::Parse(txtSigma->Text);

				imageg = new cv::Mat(image0->clone());
				cv::GaussianBlur(*image0, *imageg, cv::Size(taille, taille), sigma);

				loadMatIntoPictureBox(*imageg, 2);
			}
			catch (Exception^ ex)
			{
				MessageBox::Show(ex->Message + "\nVeuillez utiliser une virgule ',' pour les nombres réels.");
			}
		}

		// Appliquer le filtre dérivateur et enregistrer le résultat dans imagef
		private: System::Void button20_Click(System::Object^  sender, System::EventArgs^  e) {
			imagef = new cv::Mat(image0->clone());
			cv::Mat filtre;
			if (comboFPH->Text == "df/dx")
				filtre = (cv::Mat_<double>(1, 3) << 1, 0, -1);
			else if (comboFPH->Text == "df/dy")
				filtre = (cv::Mat_<double>(3, 1) << 1, 0, -1);
			else if (comboFPH->Text == "Laplacien V4")
				filtre = (cv::Mat_<double>(3, 3) << 0, 1, 0, 1, -4, 1, 0, 1, 0);
			else if (comboFPH->Text == "Laplacien V8")
				filtre = (cv::Mat_<double>(3, 3) << 1, 1, 1, 1, -8, 1, 1, 1, 1);
			else if (comboFPH->Text == "d2f/dx2")
				filtre = (cv::Mat_<double>(1, 3) << 1, -2, 1);
			else if (comboFPH->Text == "d2f/dy2")
				filtre = (cv::Mat_<double>(3, 1) << 1, -2, 1);
			else if (comboFPH->Text == "Sobel X")
				filtre = (cv::Mat_<double>(3, 3) << -1, 0, 1, -2, 0, 2, 1, 0, -1);
			else if (comboFPH->Text == "Sobel Y")
				filtre = (cv::Mat_<double>(3, 3) << -1, -2, -1, 0, 0, 0, 1, 2, 1);
			else
				return;

			filter2D(*image0, *imagef, -1, filtre, cv::Point(-1, -1), 0, cv::BORDER_DEFAULT);
			loadMatIntoPictureBox(*imagef, 3);
		}

		// Appliquer le filtre darivateur sur Ig = I*G
		private: System::Void button1_Click_1(System::Object^  sender, System::EventArgs^  e) {
			if (imageg == NULL)
			{
				MessageBox::Show("Veuillez appliquer le Guassien d'abord");
				return;
			}

			cv::Mat filtre;
			if (comboFPH->Text == "df/dx")
				filtre = (cv::Mat_<double>(1, 3) << 1, 0, -1);
			else if (comboFPH->Text == "df/dy")
				filtre = (cv::Mat_<double>(3, 1) << 1, 0, -1);
			else if (comboFPH->Text == "Laplacien V4")
				filtre = (cv::Mat_<double>(3, 3) << 0, 1, 0, 1, -4, 1, 0, 1, 0);
			else if (comboFPH->Text == "Laplacien V8")
				filtre = (cv::Mat_<double>(3, 3) << 1, 1, 1, 1, -8, 1, 1, 1, 1);
			else if (comboFPH->Text == "d2f/dx2")
				filtre = (cv::Mat_<double>(1, 3) << 1, -2, 1);
			else if (comboFPH->Text == "d2f/dy2")
				filtre = (cv::Mat_<double>(3, 1) << 1, -2, 1);
			else if (comboFPH->Text == "Sobel X")
				filtre = (cv::Mat_<double>(3, 3) << -1, 0, 1, -2, 0, 2, -1, 0, 1);
			else if (comboFPH->Text == "Sobel Y")
				filtre = (cv::Mat_<double>(3, 3) << -1, -2, -1, 0, 0, 0, 1, 2, 1);
			else
				return;

			cv::Mat Resultat;
			cv::filter2D(*imageg, Resultat, -1, filtre, cv::Point(-1, -1), 0, cv::BORDER_DEFAULT);
			loadMatIntoPictureBox(Resultat, 4);
		}

		// Calculer I - Ig
		private: System::Void button4_Click_1(System::Object^  sender, System::EventArgs^  e) {
			if (imageg)
			{
				cv::Mat imgr = (*image0) - (*imageg);
				loadMatIntoPictureBox(imgr, 4);
			}
			else
				MessageBox::Show("Veuillez calculer le Gaussien d'abord.");
		}

		// Calculer et fficher Ix, Ix2 et Ixy
		private: System::Void button5_Click_1(System::Object^  sender, System::EventArgs^  e) { 
			bool filtrer = checkboxFiltrer->Checked;

			cv::Mat img = image0->clone();
			if (filtrer) cv::GaussianBlur(img, img, cv::Size(3, 3), 1);
			cv::Mat Ix, Iy, Ix2, Iy2, Ixy;

			/*  filtre = (cv::Mat_<double>(1, 3) << 1, 0, -1);
				filtre = (cv::Mat_<double>(3, 1) << 1, 0, -1);  */

			// Sobel X et Sobel Y
			cv::Mat fx = (cv::Mat_<double>(3, 3) << -1, 0, 1, -2, 0, 2, 1, 0, -1);
			cv::Mat fy = (cv::Mat_<double>(3, 3) << -1, -2, -1, 0, 0, 0, 1, 2, 1);

			filter2D(img, Ix, -1, fx, cv::Point(-1, -1), 0, cv::BORDER_DEFAULT);
			filter2D(Ix, Ix2, -1, fx, cv::Point(-1, -1), 0, cv::BORDER_DEFAULT);
			if (filtrer) cv::GaussianBlur(Ix2, Ix2, cv::Size(3, 3), 1);
			filter2D(img, Iy, -1, fy, cv::Point(-1, -1), 0, cv::BORDER_DEFAULT);
			filter2D(Iy, Iy2, -1, fy, cv::Point(-1, -1), 0, cv::BORDER_DEFAULT);
			if (filtrer) cv::GaussianBlur(Iy2, Iy2, cv::Size(3, 3), 1);

			/*
			for (int i = 0; i < img.rows; i++)
				for (int j = 0; j < img.cols; j++)
				{
					Ixy.at<cv::Vec3b>(i, j)[0] = Ix.at<cv::Vec3b>(i, j)[0] * Iy.at<cv::Vec3b>(i, j)[0];
					Ixy.at<cv::Vec3b>(i, j)[1] = Ix.at<cv::Vec3b>(i, j)[1] * Iy.at<cv::Vec3b>(i, j)[1];
					Ixy.at<cv::Vec3b>(i, j)[2] = Ix.at<cv::Vec3b>(i, j)[2] * Iy.at<cv::Vec3b>(i, j)[2];
				}
			*/
			Ixy = Ix.mul(Iy);
			if (filtrer) cv::GaussianBlur(Ixy, Ixy, cv::Size(3, 3), 1);

			loadMatIntoPictureBox(Ix, 3);
			loadMatIntoPictureBox(Ix2, 2);
			loadMatIntoPictureBox(Ixy, 4);
		}

		// Calculer et Afficher Iy, Iy2 et Ixy
		private: System::Void button6_Click_2(System::Object^  sender, System::EventArgs^  e) {
			bool filtrer = checkboxFiltrer->Checked;

			cv::Mat img = image0->clone();
			if (filtrer) cv::GaussianBlur(img, img, cv::Size(3, 3), 1);
			cv::Mat Ix, Iy, Ix2, Iy2, Ixy;

			// Sobel X et Sobel Y
			cv::Mat fx = (cv::Mat_<double>(3, 3) << -1, 0, 1, -2, 0, 2, 1, 0, -1);
			cv::Mat fy = (cv::Mat_<double>(3, 3) << -1, -2, -1, 0, 0, 0, 1, 2, 1);

			filter2D(img, Ix, -1, fx, cv::Point(-1, -1), 0, cv::BORDER_DEFAULT);
			filter2D(Ix, Ix2, -1, fx, cv::Point(-1, -1), 0, cv::BORDER_DEFAULT);
			if (filtrer) cv::GaussianBlur(Ix2, Ix2, cv::Size(3, 3), 1);
			filter2D(img, Iy, -1, fy, cv::Point(-1, -1), 0, cv::BORDER_DEFAULT);
			filter2D(Iy, Iy2, -1, fy, cv::Point(-1, -1), 0, cv::BORDER_DEFAULT);
			if (filtrer) cv::GaussianBlur(Iy2, Iy2, cv::Size(3, 3), 1);

			Ixy = Ix.mul(Iy);
			if (filtrer) cv::GaussianBlur(Ixy, Ixy, cv::Size(3, 3), 1);

			loadMatIntoPictureBox(Iy, 3);
			loadMatIntoPictureBox(Iy2, 2);
			loadMatIntoPictureBox(Ixy, 4);
		}

		// Calculer la matrice de Harris et Afficher le résultat (avec ou sans aperçu)
		private: System::Void button7_Click_1(System::Object^  sender, System::EventArgs^  e) {
			try
			{
				float thresh = float::Parse(textSeuil->Text);
				float k = float::Parse(textBoxK->Text);
				bool filtrer = checkboxFiltrer->Checked;
				bool domination = checkBox0->Checked;

				cv::Mat H = calculerHarris(image0, filtrer, domination, thresh, k);
				
				imager = new cv::Mat(H.clone());
				loadMatIntoPictureBox(H, 4);
			}
			catch (Exception^ ex)
			{
				MessageBox::Show(ex->Message + "\nVeuillez utiliser une virgule ',' pour les nombres réels.");
			}
		}
		
		// Activer ou désactiver le Pixel Dominant
		private: System::Void checkBox9_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
			checkBox1->Enabled = checkBox0->Checked;
			checkBox2->Enabled = checkBox0->Checked;
			checkBox3->Enabled = checkBox0->Checked;
			checkBox4->Enabled = checkBox0->Checked;
			checkBox5->Enabled = checkBox0->Checked;
			checkBox6->Enabled = checkBox0->Checked;
			checkBox7->Enabled = checkBox0->Checked;
			checkBox8->Enabled = checkBox0->Checked;
		}

		// Afficher le résultat de H dans une fenêtre à part
		private: System::Void pictureBox4_Click(System::Object^  sender, System::EventArgs^  e) {
			if (imager)
			{
				std::string titre = marshal_as<std::string>("H de " + openFileDialog1->SafeFileName);
				cv::imshow(titre, *imager);
			}
		}

		// Afficher l'image avec des cercles autour des points d'intérêt
		private: System::Void pictureBox3_Click(System::Object^  sender, System::EventArgs^  e) {
			if (imagep)
			{
				std::string titre = marshal_as<std::string>("Résultat de " + openFileDialog1->SafeFileName);
				cv::imshow(titre, *imagep);
			}
		}

		// Calculer la carte de profondeur
		private: System::Void button2_Click(System::Object^  sender, System::EventArgs^  e) {
			// Récupérer les paramètres
			int w, P, N, methode;
			try
			{
				w = (int)numericW->Value;
				P = (int)numericP->Value;
				N = (int)numericN->Value;
				if (N % 2 == 0 || P % 2 == 0)
					throw gcnew Exception("N et P doivent être des nombres impairs");
				else
				{
					P /= 2;
					N /= 2;
				}
				if (comboBoxMethode->Text == "SAD") methode = 1;
				else if (comboBoxMethode->Text == "SSD") methode = 2;
				else if (comboBoxMethode->Text == "Corrélation") methode = 3;
				else throw gcnew Exception("Méthode invalide");

			}
			catch (Exception^ Ex)
			{
				MessageBox::Show(Ex->Message);
				return;
			}

			// Ouvrir les deux images L et R
			cv::Mat img1, img2;
			if (ouvrirImagesLR(&img1, &img2) == false)
				return;

			cv::cvtColor(img1, img1, CV_BGR2GRAY);
			cv::cvtColor(img2, img2, CV_BGR2GRAY);

			// Créer deux matrices de la même taille que img1 en niveaux de gris
			cv::Mat img3 = img1.clone(), img4 = img1.clone();
			normalize(img3, img3, 0, 255, cv::NORM_MINMAX, CV_32FC1, cv::Mat());
			normalize(img4, img4, 0, 255, cv::NORM_MINMAX, CV_32FC1, cv::Mat());

			// Balayage des images pour construire la carte de profondeur
			for (int i = 0, imax = img1.rows - 1; i < img1.rows; i++)
				for (int j = 0, jmax = img1.cols - 1; j < img1.cols; j++)
				{
					int d = 0, dmin = 65000, kmin = j + w/2;
					for (int k = j; k < j + w && k + w <= jmax; k++)
					{
						d = 0;
						for (int u = i - N; u <= i + N; u++)
							for (int v = j - P, v2 = k - P; v <= j + P && v2 <= k + P; v++, v2++)
								if (u >= 0 && u <= imax && v >= 0 && v <= jmax && v2 >= 0 && v2 <= jmax)
									if (methode == 1)
										d += abs(img1.at<char>(u, v2) - img2.at<char>(u, v));
									else if (methode == 2)
										d += pow(img1.at<char>(u, v2) - img2.at<char>(u, v), 2);
									else if (methode == 3)
										d += img1.at<char>(u, v2) * img2.at<char>(u, v);

						if (d < dmin)
						{
							dmin = d;
							kmin = k;
						}
					}
					if (j + w > jmax || kmin + w > jmax || kmin < 0) {
						img3.at<float>(i, j) = 0;
						img4.at<float>(i, j) = 0;
					}
					else {
						img3.at<float>(i, j) = -(j - kmin);
						img4.at<float>(i, j) = dmin;
					}
				}

			cv::Mat carteProfondeur(cv::Size(img3.cols - w, img3.rows), img3.type());
			cv::Mat carteSilimarite(cv::Size(img3.cols - w, img3.rows), img3.type());

			for (int i = 0; i < img3.rows; i++)
				for (int j = 0; j < img3.cols - w; j++)
				{
					carteProfondeur.at<float>(i, j) = img3.at<float>(i, j);
					carteSilimarite.at<float>(i, j) = img4.at<float>(i, j);
				}

			normalize(img3, img3, 0, 255, cv::NORM_MINMAX, CV_32FC1, cv::Mat());
			convertScaleAbs(img3, img3);
			cv::cvtColor(img3, img3, CV_GRAY2BGR);

			normalize(img4, img4, 0, 255, cv::NORM_MINMAX, CV_32FC1, cv::Mat());
			convertScaleAbs(img4, img4);

			cv::cvtColor(img4, img4, CV_GRAY2BGR);
			cv::medianBlur(img3, img3, 7);
			imager = new cv::Mat(img3);

			loadMatIntoPictureBox(img3, 4);
			loadMatIntoPictureBox(img4, 2);
		}

		// Mise en correspondance (CC TP)
		private: System::Void button8_Click(System::Object^  sender, System::EventArgs^  e) {
			cv::Mat imgL, imgR, imgLCercles, imgRCercles, imgFinal, imgCorr, carteCorr;
			if (ouvrirImagesLR(&imgL, &imgR) == false)
				return;
			imgFinal = imgL.clone();
			imgCorr = imgL.clone() * 0.1;
			carteCorr = imgL.clone() * 0;

			cv::cvtColor(carteCorr, carteCorr, CV_BGR2GRAY);
			normalize(carteCorr, carteCorr, 0, 255, cv::NORM_MINMAX, CV_32FC1, cv::Mat());

			try
			{
				float thresh = float::Parse(textSeuil->Text);
				float k = float::Parse(textBoxK->Text);
				bool filtrer = checkboxFiltrer->Checked;
				bool domination = checkBox0->Checked;
				int nmax = (int)numericUpDownNMax->Value;
				int limCorrespondance = 20;

				std::vector<Pt> ptsL = calculerHarris2(&imgL, filtrer, domination, thresh, k);
				imgLCercles = imagep->clone();
				std::vector<Pt> ptsR = calculerHarris2(&imgR, filtrer, domination, thresh, k);
				imgRCercles = imagep->clone();
				std::vector<bool> corrRTrouve(ptsR.size(), false);

				loadMatIntoPictureBox(imgLCercles, 1);
				loadMatIntoPictureBox(imgRCercles, 3);

				int N = 10, P = 10;

				int imax = imgL.rows - 1;
				int jmax = imgR.cols - 1;

				cv::cvtColor(imgL, imgL, CV_BGR2GRAY);
				cv::cvtColor(imgR, imgR, CV_BGR2GRAY);

				// MessageBox::Show("" + ptsL.size());

				for(int k1 = 0; k1 < ptsL.size() && k1 < nmax; k1++)
				{
					int d = 0, dmin = 60000, kmin = -1;
					int np = 0; // nombre de points du voisinage

					for (int k2 = 0; k2 < ptsL.size() && k2 < nmax; k2++)
					{
						int iL = ptsL[k1].i;
						int jL = ptsL[k1].j;
						int iR = ptsR[k2].i;
						int jR = ptsR[k2].j;
						d = 0;
						np = 0;

						if (abs(iL - iR) > 30 || abs(jL - jR) > 10)
							continue;

						for (int u = -N; u <= N; u++)
							for (int v = -P; v <= P; v++)
								if (iL + u >= 0 && iL + u < jmax && jL + v >= 0 && jL + v <= imax
								 && iR + u >= 0 && iR + u < jmax && jR + v >= 0 && jR + v <= imax)
								{
									d += abs(imgL.at<char>(jL + v, iL + u) - imgR.at<char>(jR + v, iR + u));
									np++;
								}

						if (np > 0)
						{
							d /= np;
							if (d < dmin && abs(iL - iR) < limCorrespondance && abs(jL - jR) < limCorrespondance)
							{
								dmin = d;
								kmin = k2;
								carteCorr.at<float>(jL, iL) = d;
							}
						}
					}

					if(kmin > -1)
					{
						corrRTrouve[kmin] = true;
						circle(imgFinal, cv::Point(ptsR[kmin].i, ptsR[kmin].j), 3, cv::Scalar(20, 250, 250), 1, 2, 0);
						line(imgFinal, cv::Point(ptsL[k1].i, ptsL[k1].j),
							cv::Point(ptsR[kmin].i, ptsR[kmin].j),
							cv::Scalar(20, 250, 250), 2, 8, 0);

						circle(imgCorr, cv::Point(ptsR[kmin].i, ptsR[kmin].j), 3, cv::Scalar(20, 250, 250), 1, 2, 0);
						line(imgCorr, cv::Point(ptsL[k1].i, ptsL[k1].j),
							cv::Point(ptsR[kmin].i, ptsR[kmin].j),
							cv::Scalar(20, 250, 250), 2, 8, 0);

						loadMatIntoPictureBox(imgCorr, 4); 
					}

					else
					{
						circle(imgCorr, cv::Point(ptsL[k1].i, ptsL[k1].j), 3, cv::Scalar(20, 20, 250), 1, 2, 0);
					}
				}

				/*
				// Entourer en bleu les points de R qui n'ont pas de correspondant
				for (int k = 0; k < nmax; k++)
					if (corrRTrouve[k] == false)
					{
						circle(imgCorr, cv::Point(ptsR[k].i, ptsR[k].j), 3, cv::Scalar(250, 20, 20), 1, 2, 0);
					}
				*/

				imager = new cv::Mat(imgCorr.clone());

				normalize(carteCorr, carteCorr, 0, 255, cv::NORM_MINMAX, CV_32FC1, cv::Mat());
				convertScaleAbs(carteCorr, carteCorr);
				cv::cvtColor(carteCorr, carteCorr, CV_GRAY2BGR);

				loadMatIntoPictureBox(carteCorr, 2);
				loadMatIntoPictureBox(imgCorr, 4);
			}
			catch (Exception^ ex)
			{
				MessageBox::Show(ex->Message + "\nException quelque part.");
			}
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
	private: System::Windows::Forms::OpenFileDialog^  openFileDialog1;
	private: System::Windows::Forms::OpenFileDialog^  openFileDialog2;
	private: System::Windows::Forms::PictureBox^  pictureBox2;
	private: System::Windows::Forms::PictureBox^  pictureBox4;
	private: System::Windows::Forms::PictureBox^  pictureBox3;
	private: System::Windows::Forms::OpenFileDialog^  openFileDialog3;
	private: System::Windows::Forms::TextBox^  textBoxK;
	private: System::Windows::Forms::Label^  labelTrouves;
	private: System::Windows::Forms::Label^  label2;
	private: System::Windows::Forms::GroupBox^  groupBox2;
	private: System::Windows::Forms::CheckBox^  checkBox0;
	private: System::Windows::Forms::CheckBox^  checkBox8;
	private: System::Windows::Forms::CheckBox^  checkBox7;
	private: System::Windows::Forms::CheckBox^  checkBox6;
	private: System::Windows::Forms::CheckBox^  checkBox5;
	private: System::Windows::Forms::CheckBox^  checkBox4;
	private: System::Windows::Forms::CheckBox^  checkBox3;
	private: System::Windows::Forms::CheckBox^  checkBox2;
	private: System::Windows::Forms::CheckBox^  checkBox1;
	private: System::Windows::Forms::Label^  label5;
	private: System::Windows::Forms::Label^  label1;
	private: System::Windows::Forms::NumericUpDown^  numericUpDownNMax;
	private: System::Windows::Forms::Button^  button7;
	private: System::Windows::Forms::CheckBox^  checkboxFiltrer;
	private: System::Windows::Forms::Label^  labelAffiches;
	private: System::Windows::Forms::TextBox^  textSeuil;
	private: System::Windows::Forms::GroupBox^  groupBox3;
	private: System::Windows::Forms::Button^  button2;
	private: System::Windows::Forms::Label^  label6;
	private: System::Windows::Forms::ComboBox^  comboBoxMethode;
	private: System::Windows::Forms::NumericUpDown^  numericW;
	private: System::Windows::Forms::Label^  label7;
	private: System::Windows::Forms::NumericUpDown^  numericN;
	private: System::Windows::Forms::Label^  label8;
	private: System::Windows::Forms::NumericUpDown^  numericP;
	private: System::Windows::Forms::Label^  label9;
	private: System::Windows::Forms::GroupBox^  groupBox4;
	private: System::Windows::Forms::Button^  button20;
	private: System::Windows::Forms::ComboBox^  comboFPH;
	private: System::Windows::Forms::Button^  button1;
	private: System::Windows::Forms::TextBox^  txtSigma;
	private: System::Windows::Forms::Button^  button4;
	private: System::Windows::Forms::Label^  label4;
	private: System::Windows::Forms::Button^  button5;
	private: System::Windows::Forms::NumericUpDown^  numericNGauss;
	private: System::Windows::Forms::Button^  button6;
	private: System::Windows::Forms::Label^  label3;
	private: System::Windows::Forms::Button^  button3;
	private: System::Windows::Forms::Button^  buttonLoad;
	private: System::Windows::Forms::GroupBox^  groupBox5;
	private: System::Windows::Forms::Button^  button8;
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
			this->pictureBox1 = (gcnew System::Windows::Forms::PictureBox());
			this->openFileDialog1 = (gcnew System::Windows::Forms::OpenFileDialog());
			this->openFileDialog2 = (gcnew System::Windows::Forms::OpenFileDialog());
			this->pictureBox2 = (gcnew System::Windows::Forms::PictureBox());
			this->pictureBox4 = (gcnew System::Windows::Forms::PictureBox());
			this->pictureBox3 = (gcnew System::Windows::Forms::PictureBox());
			this->openFileDialog3 = (gcnew System::Windows::Forms::OpenFileDialog());
			this->textBoxK = (gcnew System::Windows::Forms::TextBox());
			this->labelTrouves = (gcnew System::Windows::Forms::Label());
			this->label2 = (gcnew System::Windows::Forms::Label());
			this->groupBox2 = (gcnew System::Windows::Forms::GroupBox());
			this->checkBox0 = (gcnew System::Windows::Forms::CheckBox());
			this->checkBox8 = (gcnew System::Windows::Forms::CheckBox());
			this->checkBox7 = (gcnew System::Windows::Forms::CheckBox());
			this->checkBox6 = (gcnew System::Windows::Forms::CheckBox());
			this->checkBox5 = (gcnew System::Windows::Forms::CheckBox());
			this->checkBox4 = (gcnew System::Windows::Forms::CheckBox());
			this->checkBox3 = (gcnew System::Windows::Forms::CheckBox());
			this->checkBox2 = (gcnew System::Windows::Forms::CheckBox());
			this->checkBox1 = (gcnew System::Windows::Forms::CheckBox());
			this->label5 = (gcnew System::Windows::Forms::Label());
			this->label1 = (gcnew System::Windows::Forms::Label());
			this->numericUpDownNMax = (gcnew System::Windows::Forms::NumericUpDown());
			this->button7 = (gcnew System::Windows::Forms::Button());
			this->checkboxFiltrer = (gcnew System::Windows::Forms::CheckBox());
			this->labelAffiches = (gcnew System::Windows::Forms::Label());
			this->textSeuil = (gcnew System::Windows::Forms::TextBox());
			this->groupBox3 = (gcnew System::Windows::Forms::GroupBox());
			this->button2 = (gcnew System::Windows::Forms::Button());
			this->label6 = (gcnew System::Windows::Forms::Label());
			this->comboBoxMethode = (gcnew System::Windows::Forms::ComboBox());
			this->numericW = (gcnew System::Windows::Forms::NumericUpDown());
			this->label7 = (gcnew System::Windows::Forms::Label());
			this->numericN = (gcnew System::Windows::Forms::NumericUpDown());
			this->label8 = (gcnew System::Windows::Forms::Label());
			this->numericP = (gcnew System::Windows::Forms::NumericUpDown());
			this->label9 = (gcnew System::Windows::Forms::Label());
			this->groupBox4 = (gcnew System::Windows::Forms::GroupBox());
			this->button20 = (gcnew System::Windows::Forms::Button());
			this->comboFPH = (gcnew System::Windows::Forms::ComboBox());
			this->button1 = (gcnew System::Windows::Forms::Button());
			this->txtSigma = (gcnew System::Windows::Forms::TextBox());
			this->button4 = (gcnew System::Windows::Forms::Button());
			this->label4 = (gcnew System::Windows::Forms::Label());
			this->button5 = (gcnew System::Windows::Forms::Button());
			this->numericNGauss = (gcnew System::Windows::Forms::NumericUpDown());
			this->button6 = (gcnew System::Windows::Forms::Button());
			this->label3 = (gcnew System::Windows::Forms::Label());
			this->button3 = (gcnew System::Windows::Forms::Button());
			this->buttonLoad = (gcnew System::Windows::Forms::Button());
			this->groupBox5 = (gcnew System::Windows::Forms::GroupBox());
			this->button8 = (gcnew System::Windows::Forms::Button());
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pictureBox1))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pictureBox2))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pictureBox4))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pictureBox3))->BeginInit();
			this->groupBox2->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numericUpDownNMax))->BeginInit();
			this->groupBox3->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numericW))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numericN))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numericP))->BeginInit();
			this->groupBox4->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numericNGauss))->BeginInit();
			this->groupBox5->SuspendLayout();
			this->SuspendLayout();
			// 
			// pictureBox1
			// 
			this->pictureBox1->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
			this->pictureBox1->Location = System::Drawing::Point(302, 20);
			this->pictureBox1->Name = L"pictureBox1";
			this->pictureBox1->Size = System::Drawing::Size(350, 300);
			this->pictureBox1->TabIndex = 4;
			this->pictureBox1->TabStop = false;
			// 
			// openFileDialog1
			// 
			this->openFileDialog1->FileName = L"openFileDialog1";
			// 
			// openFileDialog2
			// 
			this->openFileDialog2->FileName = L"openFileDialog2";
			// 
			// pictureBox2
			// 
			this->pictureBox2->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
			this->pictureBox2->Location = System::Drawing::Point(302, 336);
			this->pictureBox2->Name = L"pictureBox2";
			this->pictureBox2->Size = System::Drawing::Size(350, 300);
			this->pictureBox2->SizeMode = System::Windows::Forms::PictureBoxSizeMode::StretchImage;
			this->pictureBox2->TabIndex = 9;
			this->pictureBox2->TabStop = false;
			// 
			// pictureBox4
			// 
			this->pictureBox4->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
			this->pictureBox4->Location = System::Drawing::Point(664, 336);
			this->pictureBox4->Name = L"pictureBox4";
			this->pictureBox4->Size = System::Drawing::Size(350, 300);
			this->pictureBox4->SizeMode = System::Windows::Forms::PictureBoxSizeMode::StretchImage;
			this->pictureBox4->TabIndex = 11;
			this->pictureBox4->TabStop = false;
			this->pictureBox4->Click += gcnew System::EventHandler(this, &MyForm::pictureBox4_Click);
			// 
			// pictureBox3
			// 
			this->pictureBox3->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
			this->pictureBox3->Location = System::Drawing::Point(664, 20);
			this->pictureBox3->Name = L"pictureBox3";
			this->pictureBox3->Size = System::Drawing::Size(350, 300);
			this->pictureBox3->SizeMode = System::Windows::Forms::PictureBoxSizeMode::StretchImage;
			this->pictureBox3->TabIndex = 10;
			this->pictureBox3->TabStop = false;
			this->pictureBox3->Click += gcnew System::EventHandler(this, &MyForm::pictureBox3_Click);
			// 
			// openFileDialog3
			// 
			this->openFileDialog3->FileName = L"openFileDialog3";
			// 
			// textBoxK
			// 
			this->textBoxK->Location = System::Drawing::Point(77, 46);
			this->textBoxK->Name = L"textBoxK";
			this->textBoxK->Size = System::Drawing::Size(58, 20);
			this->textBoxK->TabIndex = 65;
			this->textBoxK->Text = L"0,04";
			// 
			// labelTrouves
			// 
			this->labelTrouves->AutoSize = true;
			this->labelTrouves->Location = System::Drawing::Point(45, 169);
			this->labelTrouves->Name = L"labelTrouves";
			this->labelTrouves->Size = System::Drawing::Size(123, 13);
			this->labelTrouves->TabIndex = 67;
			this->labelTrouves->Text = L"Points d\'intérêt trouvés : ";
			// 
			// label2
			// 
			this->label2->AutoSize = true;
			this->label2->Location = System::Drawing::Point(38, 50);
			this->label2->Name = L"label2";
			this->label2->Size = System::Drawing::Size(13, 13);
			this->label2->TabIndex = 64;
			this->label2->Text = L"k";
			// 
			// groupBox2
			// 
			this->groupBox2->Controls->Add(this->checkBox0);
			this->groupBox2->Controls->Add(this->checkBox8);
			this->groupBox2->Controls->Add(this->checkBox7);
			this->groupBox2->Controls->Add(this->checkBox6);
			this->groupBox2->Controls->Add(this->checkBox5);
			this->groupBox2->Controls->Add(this->checkBox4);
			this->groupBox2->Controls->Add(this->checkBox3);
			this->groupBox2->Controls->Add(this->checkBox2);
			this->groupBox2->Controls->Add(this->checkBox1);
			this->groupBox2->Location = System::Drawing::Point(167, 17);
			this->groupBox2->Name = L"groupBox2";
			this->groupBox2->Size = System::Drawing::Size(79, 79);
			this->groupBox2->TabIndex = 62;
			this->groupBox2->TabStop = false;
			this->groupBox2->Text = L"Dominance";
			// 
			// checkBox0
			// 
			this->checkBox0->AutoSize = true;
			this->checkBox0->BackColor = System::Drawing::SystemColors::ActiveCaption;
			this->checkBox0->Checked = true;
			this->checkBox0->CheckState = System::Windows::Forms::CheckState::Checked;
			this->checkBox0->Location = System::Drawing::Point(33, 37);
			this->checkBox0->Name = L"checkBox0";
			this->checkBox0->Size = System::Drawing::Size(15, 14);
			this->checkBox0->TabIndex = 8;
			this->checkBox0->UseVisualStyleBackColor = false;
			this->checkBox0->CheckedChanged += gcnew System::EventHandler(this, &MyForm::checkBox9_CheckedChanged);
			// 
			// checkBox8
			// 
			this->checkBox8->AutoSize = true;
			this->checkBox8->Checked = true;
			this->checkBox8->CheckState = System::Windows::Forms::CheckState::Checked;
			this->checkBox8->Location = System::Drawing::Point(54, 57);
			this->checkBox8->Name = L"checkBox8";
			this->checkBox8->Size = System::Drawing::Size(15, 14);
			this->checkBox8->TabIndex = 7;
			this->checkBox8->UseVisualStyleBackColor = true;
			// 
			// checkBox7
			// 
			this->checkBox7->AutoSize = true;
			this->checkBox7->Checked = true;
			this->checkBox7->CheckState = System::Windows::Forms::CheckState::Checked;
			this->checkBox7->Location = System::Drawing::Point(33, 57);
			this->checkBox7->Name = L"checkBox7";
			this->checkBox7->Size = System::Drawing::Size(15, 14);
			this->checkBox7->TabIndex = 6;
			this->checkBox7->UseVisualStyleBackColor = true;
			// 
			// checkBox6
			// 
			this->checkBox6->AutoSize = true;
			this->checkBox6->Checked = true;
			this->checkBox6->CheckState = System::Windows::Forms::CheckState::Checked;
			this->checkBox6->Location = System::Drawing::Point(13, 57);
			this->checkBox6->Name = L"checkBox6";
			this->checkBox6->Size = System::Drawing::Size(15, 14);
			this->checkBox6->TabIndex = 5;
			this->checkBox6->UseVisualStyleBackColor = true;
			// 
			// checkBox5
			// 
			this->checkBox5->AutoSize = true;
			this->checkBox5->Checked = true;
			this->checkBox5->CheckState = System::Windows::Forms::CheckState::Checked;
			this->checkBox5->Location = System::Drawing::Point(54, 37);
			this->checkBox5->Name = L"checkBox5";
			this->checkBox5->Size = System::Drawing::Size(15, 14);
			this->checkBox5->TabIndex = 4;
			this->checkBox5->UseVisualStyleBackColor = true;
			// 
			// checkBox4
			// 
			this->checkBox4->AutoSize = true;
			this->checkBox4->Checked = true;
			this->checkBox4->CheckState = System::Windows::Forms::CheckState::Checked;
			this->checkBox4->Location = System::Drawing::Point(13, 37);
			this->checkBox4->Name = L"checkBox4";
			this->checkBox4->Size = System::Drawing::Size(15, 14);
			this->checkBox4->TabIndex = 3;
			this->checkBox4->UseVisualStyleBackColor = true;
			// 
			// checkBox3
			// 
			this->checkBox3->AutoSize = true;
			this->checkBox3->Checked = true;
			this->checkBox3->CheckState = System::Windows::Forms::CheckState::Checked;
			this->checkBox3->Location = System::Drawing::Point(54, 17);
			this->checkBox3->Name = L"checkBox3";
			this->checkBox3->Size = System::Drawing::Size(15, 14);
			this->checkBox3->TabIndex = 2;
			this->checkBox3->UseVisualStyleBackColor = true;
			// 
			// checkBox2
			// 
			this->checkBox2->AutoSize = true;
			this->checkBox2->Checked = true;
			this->checkBox2->CheckState = System::Windows::Forms::CheckState::Checked;
			this->checkBox2->Location = System::Drawing::Point(33, 17);
			this->checkBox2->Name = L"checkBox2";
			this->checkBox2->Size = System::Drawing::Size(15, 14);
			this->checkBox2->TabIndex = 1;
			this->checkBox2->UseVisualStyleBackColor = true;
			// 
			// checkBox1
			// 
			this->checkBox1->AutoSize = true;
			this->checkBox1->Checked = true;
			this->checkBox1->CheckState = System::Windows::Forms::CheckState::Checked;
			this->checkBox1->Location = System::Drawing::Point(13, 17);
			this->checkBox1->Name = L"checkBox1";
			this->checkBox1->Size = System::Drawing::Size(15, 14);
			this->checkBox1->TabIndex = 0;
			this->checkBox1->UseVisualStyleBackColor = true;
			// 
			// label5
			// 
			this->label5->AutoSize = true;
			this->label5->Location = System::Drawing::Point(38, 76);
			this->label5->Name = L"label5";
			this->label5->Size = System::Drawing::Size(45, 13);
			this->label5->TabIndex = 69;
			this->label5->Text = L"Nb. Pts ";
			// 
			// label1
			// 
			this->label1->AutoSize = true;
			this->label1->Location = System::Drawing::Point(37, 25);
			this->label1->Name = L"label1";
			this->label1->Size = System::Drawing::Size(30, 13);
			this->label1->TabIndex = 59;
			this->label1->Text = L"Seuil";
			// 
			// numericUpDownNMax
			// 
			this->numericUpDownNMax->Location = System::Drawing::Point(89, 74);
			this->numericUpDownNMax->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 500, 0, 0, 0 });
			this->numericUpDownNMax->Name = L"numericUpDownNMax";
			this->numericUpDownNMax->Size = System::Drawing::Size(46, 20);
			this->numericUpDownNMax->TabIndex = 70;
			this->numericUpDownNMax->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 50, 0, 0, 0 });
			// 
			// button7
			// 
			this->button7->Location = System::Drawing::Point(126, 102);
			this->button7->Name = L"button7";
			this->button7->Size = System::Drawing::Size(118, 36);
			this->button7->TabIndex = 58;
			this->button7->Text = L"Harris";
			this->button7->UseVisualStyleBackColor = true;
			this->button7->Click += gcnew System::EventHandler(this, &MyForm::button7_Click_1);
			// 
			// checkboxFiltrer
			// 
			this->checkboxFiltrer->AutoSize = true;
			this->checkboxFiltrer->Checked = true;
			this->checkboxFiltrer->CheckState = System::Windows::Forms::CheckState::Checked;
			this->checkboxFiltrer->Location = System::Drawing::Point(41, 110);
			this->checkboxFiltrer->Name = L"checkboxFiltrer";
			this->checkboxFiltrer->Size = System::Drawing::Size(70, 17);
			this->checkboxFiltrer->TabIndex = 71;
			this->checkboxFiltrer->Text = L"Gaussien";
			this->checkboxFiltrer->UseVisualStyleBackColor = true;
			// 
			// labelAffiches
			// 
			this->labelAffiches->AutoSize = true;
			this->labelAffiches->Location = System::Drawing::Point(44, 155);
			this->labelAffiches->Name = L"labelAffiches";
			this->labelAffiches->Size = System::Drawing::Size(125, 13);
			this->labelAffiches->TabIndex = 68;
			this->labelAffiches->Text = L"Points d\'intérêt affichés : ";
			// 
			// textSeuil
			// 
			this->textSeuil->Location = System::Drawing::Point(77, 21);
			this->textSeuil->Name = L"textSeuil";
			this->textSeuil->Size = System::Drawing::Size(58, 20);
			this->textSeuil->TabIndex = 60;
			this->textSeuil->Text = L"70";
			// 
			// groupBox3
			// 
			this->groupBox3->Controls->Add(this->textSeuil);
			this->groupBox3->Controls->Add(this->labelAffiches);
			this->groupBox3->Controls->Add(this->checkboxFiltrer);
			this->groupBox3->Controls->Add(this->button7);
			this->groupBox3->Controls->Add(this->numericUpDownNMax);
			this->groupBox3->Controls->Add(this->label1);
			this->groupBox3->Controls->Add(this->label5);
			this->groupBox3->Controls->Add(this->groupBox2);
			this->groupBox3->Controls->Add(this->label2);
			this->groupBox3->Controls->Add(this->labelTrouves);
			this->groupBox3->Controls->Add(this->textBoxK);
			this->groupBox3->Location = System::Drawing::Point(12, 198);
			this->groupBox3->Name = L"groupBox3";
			this->groupBox3->Size = System::Drawing::Size(267, 199);
			this->groupBox3->TabIndex = 72;
			this->groupBox3->TabStop = false;
			this->groupBox3->Text = L"Matrice de Harris";
			// 
			// button2
			// 
			this->button2->Location = System::Drawing::Point(49, 102);
			this->button2->Name = L"button2";
			this->button2->Size = System::Drawing::Size(166, 37);
			this->button2->TabIndex = 0;
			this->button2->Text = L"Carte de Profondeur";
			this->button2->UseVisualStyleBackColor = true;
			this->button2->Click += gcnew System::EventHandler(this, &MyForm::button2_Click);
			// 
			// label6
			// 
			this->label6->AutoSize = true;
			this->label6->Location = System::Drawing::Point(46, 27);
			this->label6->Name = L"label6";
			this->label6->Size = System::Drawing::Size(58, 13);
			this->label6->TabIndex = 1;
			this->label6->Text = L"Méthode : ";
			// 
			// comboBoxMethode
			// 
			this->comboBoxMethode->FormattingEnabled = true;
			this->comboBoxMethode->Items->AddRange(gcnew cli::array< System::Object^  >(2) { L"SAD", L"SSD" });
			this->comboBoxMethode->Location = System::Drawing::Point(115, 24);
			this->comboBoxMethode->Name = L"comboBoxMethode";
			this->comboBoxMethode->Size = System::Drawing::Size(100, 21);
			this->comboBoxMethode->TabIndex = 2;
			this->comboBoxMethode->Text = L"SAD";
			// 
			// numericW
			// 
			this->numericW->Location = System::Drawing::Point(49, 72);
			this->numericW->Name = L"numericW";
			this->numericW->Size = System::Drawing::Size(44, 20);
			this->numericW->TabIndex = 74;
			this->numericW->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 18, 0, 0, 0 });
			// 
			// label7
			// 
			this->label7->AutoSize = true;
			this->label7->Location = System::Drawing::Point(46, 54);
			this->label7->Name = L"label7";
			this->label7->Size = System::Drawing::Size(53, 13);
			this->label7->TabIndex = 75;
			this->label7->Text = L"Voisinage";
			// 
			// numericN
			// 
			this->numericN->Location = System::Drawing::Point(124, 74);
			this->numericN->Name = L"numericN";
			this->numericN->Size = System::Drawing::Size(35, 20);
			this->numericN->TabIndex = 76;
			this->numericN->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 3, 0, 0, 0 });
			// 
			// label8
			// 
			this->label8->AutoSize = true;
			this->label8->Location = System::Drawing::Point(126, 56);
			this->label8->Name = L"label8";
			this->label8->Size = System::Drawing::Size(86, 13);
			this->label8->TabIndex = 77;
			this->label8->Text = L"Taille de Fenêtre";
			// 
			// numericP
			// 
			this->numericP->Location = System::Drawing::Point(179, 72);
			this->numericP->Name = L"numericP";
			this->numericP->Size = System::Drawing::Size(35, 20);
			this->numericP->TabIndex = 78;
			this->numericP->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 3, 0, 0, 0 });
			// 
			// label9
			// 
			this->label9->AutoSize = true;
			this->label9->Location = System::Drawing::Point(164, 74);
			this->label9->Name = L"label9";
			this->label9->Size = System::Drawing::Size(12, 13);
			this->label9->TabIndex = 79;
			this->label9->Text = L"x";
			// 
			// groupBox4
			// 
			this->groupBox4->Controls->Add(this->label9);
			this->groupBox4->Controls->Add(this->numericP);
			this->groupBox4->Controls->Add(this->label8);
			this->groupBox4->Controls->Add(this->numericN);
			this->groupBox4->Controls->Add(this->label7);
			this->groupBox4->Controls->Add(this->numericW);
			this->groupBox4->Controls->Add(this->comboBoxMethode);
			this->groupBox4->Controls->Add(this->label6);
			this->groupBox4->Controls->Add(this->button2);
			this->groupBox4->Location = System::Drawing::Point(13, 403);
			this->groupBox4->Name = L"groupBox4";
			this->groupBox4->Size = System::Drawing::Size(266, 149);
			this->groupBox4->TabIndex = 73;
			this->groupBox4->TabStop = false;
			this->groupBox4->Text = L"Carte de Profondeur / Mise en corresp.";
			// 
			// button20
			// 
			this->button20->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 8));
			this->button20->Location = System::Drawing::Point(113, 101);
			this->button20->Name = L"button20";
			this->button20->Size = System::Drawing::Size(65, 28);
			this->button20->TabIndex = 52;
			this->button20->Text = L"Appliquer";
			this->button20->UseVisualStyleBackColor = true;
			this->button20->Click += gcnew System::EventHandler(this, &MyForm::button20_Click);
			// 
			// comboFPH
			// 
			this->comboFPH->FormattingEnabled = true;
			this->comboFPH->Items->AddRange(gcnew cli::array< System::Object^  >(8) {
				L"df/dx", L"df/dy", L"d2f/dx2", L"d2f/dy2", L"Sobel X",
					L"Sobel Y", L"Laplacien V4", L"Laplacien V8"
			});
			this->comboFPH->Location = System::Drawing::Point(26, 106);
			this->comboFPH->Name = L"comboFPH";
			this->comboFPH->Size = System::Drawing::Size(80, 21);
			this->comboFPH->TabIndex = 51;
			this->comboFPH->Text = L"df/dx";
			// 
			// button1
			// 
			this->button1->Location = System::Drawing::Point(184, 101);
			this->button1->Name = L"button1";
			this->button1->Size = System::Drawing::Size(62, 28);
			this->button1->TabIndex = 53;
			this->button1->Text = L"d(Ig)";
			this->button1->UseVisualStyleBackColor = true;
			this->button1->Click += gcnew System::EventHandler(this, &MyForm::button1_Click_1);
			// 
			// txtSigma
			// 
			this->txtSigma->Location = System::Drawing::Point(62, 75);
			this->txtSigma->Name = L"txtSigma";
			this->txtSigma->Size = System::Drawing::Size(44, 20);
			this->txtSigma->TabIndex = 44;
			this->txtSigma->Text = L"1,5";
			// 
			// button4
			// 
			this->button4->Location = System::Drawing::Point(25, 132);
			this->button4->Name = L"button4";
			this->button4->Size = System::Drawing::Size(51, 27);
			this->button4->TabIndex = 55;
			this->button4->Text = L"I - Ig";
			this->button4->UseVisualStyleBackColor = true;
			this->button4->Click += gcnew System::EventHandler(this, &MyForm::button4_Click_1);
			// 
			// label4
			// 
			this->label4->AutoSize = true;
			this->label4->Location = System::Drawing::Point(110, 78);
			this->label4->Name = L"label4";
			this->label4->Size = System::Drawing::Size(15, 13);
			this->label4->TabIndex = 40;
			this->label4->Text = L"N";
			// 
			// button5
			// 
			this->button5->Location = System::Drawing::Point(82, 132);
			this->button5->Name = L"button5";
			this->button5->Size = System::Drawing::Size(82, 28);
			this->button5->TabIndex = 56;
			this->button5->Text = L"Ix, Ix2 et  Ixy";
			this->button5->UseVisualStyleBackColor = true;
			this->button5->Click += gcnew System::EventHandler(this, &MyForm::button5_Click_1);
			// 
			// numericNGauss
			// 
			this->numericNGauss->Location = System::Drawing::Point(126, 75);
			this->numericNGauss->Name = L"numericNGauss";
			this->numericNGauss->Size = System::Drawing::Size(30, 20);
			this->numericNGauss->TabIndex = 39;
			this->numericNGauss->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 3, 0, 0, 0 });
			// 
			// button6
			// 
			this->button6->Location = System::Drawing::Point(166, 132);
			this->button6->Name = L"button6";
			this->button6->Size = System::Drawing::Size(80, 28);
			this->button6->TabIndex = 57;
			this->button6->Text = L"Iy, Iy2 et Ixy";
			this->button6->UseVisualStyleBackColor = true;
			this->button6->Click += gcnew System::EventHandler(this, &MyForm::button6_Click_2);
			// 
			// label3
			// 
			this->label3->AutoSize = true;
			this->label3->Location = System::Drawing::Point(23, 76);
			this->label3->Name = L"label3";
			this->label3->Size = System::Drawing::Size(36, 13);
			this->label3->TabIndex = 38;
			this->label3->Text = L"Sigma";
			// 
			// button3
			// 
			this->button3->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 8));
			this->button3->Location = System::Drawing::Point(165, 70);
			this->button3->Name = L"button3";
			this->button3->Size = System::Drawing::Size(81, 28);
			this->button3->TabIndex = 27;
			this->button3->Text = L"Gaussien";
			this->button3->UseVisualStyleBackColor = true;
			this->button3->Click += gcnew System::EventHandler(this, &MyForm::button3_Click);
			// 
			// buttonLoad
			// 
			this->buttonLoad->Location = System::Drawing::Point(27, 21);
			this->buttonLoad->Name = L"buttonLoad";
			this->buttonLoad->Size = System::Drawing::Size(219, 38);
			this->buttonLoad->TabIndex = 12;
			this->buttonLoad->Text = L"Choisir une image";
			this->buttonLoad->UseVisualStyleBackColor = true;
			this->buttonLoad->Click += gcnew System::EventHandler(this, &MyForm::button6_Click);
			// 
			// groupBox5
			// 
			this->groupBox5->Controls->Add(this->buttonLoad);
			this->groupBox5->Controls->Add(this->button3);
			this->groupBox5->Controls->Add(this->label3);
			this->groupBox5->Controls->Add(this->button6);
			this->groupBox5->Controls->Add(this->numericNGauss);
			this->groupBox5->Controls->Add(this->button5);
			this->groupBox5->Controls->Add(this->label4);
			this->groupBox5->Controls->Add(this->button4);
			this->groupBox5->Controls->Add(this->txtSigma);
			this->groupBox5->Controls->Add(this->button1);
			this->groupBox5->Controls->Add(this->comboFPH);
			this->groupBox5->Controls->Add(this->button20);
			this->groupBox5->Location = System::Drawing::Point(12, 20);
			this->groupBox5->Name = L"groupBox5";
			this->groupBox5->Size = System::Drawing::Size(267, 172);
			this->groupBox5->TabIndex = 75;
			this->groupBox5->TabStop = false;
			this->groupBox5->Text = L"Visualisation";
			// 
			// button8
			// 
			this->button8->Location = System::Drawing::Point(39, 568);
			this->button8->Name = L"button8";
			this->button8->Size = System::Drawing::Size(226, 49);
			this->button8->TabIndex = 76;
			this->button8->Text = L"Mise en correspondance";
			this->button8->UseVisualStyleBackColor = true;
			this->button8->Click += gcnew System::EventHandler(this, &MyForm::button8_Click);
			// 
			// MyForm
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->BackColor = System::Drawing::SystemColors::ButtonHighlight;
			this->ClientSize = System::Drawing::Size(1027, 646);
			this->Controls->Add(this->button8);
			this->Controls->Add(this->groupBox5);
			this->Controls->Add(this->groupBox4);
			this->Controls->Add(this->pictureBox4);
			this->Controls->Add(this->groupBox3);
			this->Controls->Add(this->pictureBox3);
			this->Controls->Add(this->pictureBox2);
			this->Controls->Add(this->pictureBox1);
			this->Name = L"MyForm";
			this->Text = L"TP2 - Vision Numérique";
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pictureBox1))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pictureBox2))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pictureBox4))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pictureBox3))->EndInit();
			this->groupBox2->ResumeLayout(false);
			this->groupBox2->PerformLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numericUpDownNMax))->EndInit();
			this->groupBox3->ResumeLayout(false);
			this->groupBox3->PerformLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numericW))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numericN))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numericP))->EndInit();
			this->groupBox4->ResumeLayout(false);
			this->groupBox4->PerformLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numericNGauss))->EndInit();
			this->groupBox5->ResumeLayout(false);
			this->groupBox5->PerformLayout();
			this->ResumeLayout(false);

		}
	#pragma endregion
};
}
