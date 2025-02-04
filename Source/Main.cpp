#include <iostream>
#include <Gorgon/Main.h>
#include <Gorgon/Window.h>
#include <Gorgon/WindowManager.h>
#include <Gorgon/Graphics/Layer.h>
#include <Gorgon/Graphics/FreeType.h>
#include <Gorgon/Graphics/BlankImage.h>
#include <Gorgon/Input/Keyboard.h>

class Histogram {
public:
	Histogram(const Gorgon::Graphics::Bitmap &tsource):source(tsource.Duplicate()){
		SetMaxMin();
		PrepareHistogram();
		SetPdf();
		SetCdf();

	}
	int GetMin(int channel=0) {
		auto min = source(0, 0, channel);
		for (int y = 1; y < source.GetHeight(); y++) {
			for (int x = 1; x < source.GetWidth(); x++) {
				min = source(x, y, channel) < min ? source(x, y, channel) : min;

			}

		}
		return min;

	}
	int GetMax(int channel = 0) {
		auto max = source(0, 0, channel);
		for (int y = 1; y < source.GetHeight(); y++) {
			for (int x = 1; x < source.GetWidth(); x++) {
				max = source(x, y, channel) > max ? source(x, y, channel) : max;

			}

		}
		return max;
	}
	void SetMaxMin() {
		//check no channel
		int c = source.GetMode() != Gorgon::Graphics::ColorMode::Grayscale ? 3 : 1;
			for (int channel = 0; channel < c; channel++) {
				min[channel] = GetMin(channel);
				max[channel] = GetMax(channel);

			}

	}
	Gorgon::Graphics::Bitmap ContastStrechingColorFullImage()const {
		Gorgon::Graphics::Bitmap streched = source.Duplicate();
		// check no channel 
		
		int nochannel = source.GetMode() != Gorgon::Graphics::ColorMode::Grayscale? 3 : 1;

		for (int y = 0; y < source.GetHeight(); y++) {
			for (int x = 0; x < source.GetWidth(); x++) {
			
				for (int c = 0; c < nochannel; c++) {
					// we are working with 256 bit image thats why our depthbit = 8
					float delta = (std::pow(2, 8) - 1)/(max[c]-min[c]);
					streched({ x,y }, c) = (source({ x,y }, c) - min[c])*delta;

				}

			}
		}

		return streched;
	}
	void PrepareHistogram() {
		int nochannel = source.GetMode() != Gorgon::Graphics::ColorMode::Grayscale ? 3 : 1;

		for (int y = 0; y < source.GetHeight(); y++) {
			for (int x = 0; x < source.GetWidth(); x++) {
				//calculate each channel for each histogram
				for (int c = 0; c < nochannel; c++) {
					int value = source(x, y, c);
					histogram[c][value] += 1;
				}



			}
		}
		

	}

	void SetPdf() {
		int nochannel = source.GetMode() != Gorgon::Graphics::ColorMode::Grayscale ? 3 : 1;

		int numberofpixel= source.GetWidth()*source.GetHeight();
		for (int i = 0; i < 256; i++) {
			//calculate each channel pdf value
			for(int c=0;c<nochannel;c++)
				pdf[c][i] = (float)histogram[c][i] / numberofpixel;


		}


	}
	void SetCdf() {
		int nochannel = source.GetMode() != Gorgon::Graphics::ColorMode::Grayscale ? 3 : 1;

		float result[3] = { 0,0,0 };
		for (int i = 0; i < 256; i++) {
			// calculate each channel cdf from each channel pdf
			for (int c = 0; c < nochannel; c++) {
				if (pdf[c][i] != 0) {
					result[c] += pdf[c][i];
					cdf[c][i] = result[c];

				}
				else
					cdf[c][i] = 0;
			}
		}

	}
	Gorgon::Graphics::Bitmap HistogramEqualization()const {
		Gorgon::Graphics::Bitmap equalized = source.Duplicate();
		int nochannel = source.GetMode() != Gorgon::Graphics::ColorMode::Grayscale ? 3 : 1;

		//we made 2^ 8 because our bitdepth = 8 we will get highest intesity
		int hightest_intensity = std::pow(2, 8) - 1;
		//we have 3 channel we need to implement all algorithm for 3 channel and we have different values
		int lastresult[3][256];
		for (int i = 0; i < 256; i++) {
			// caclulate result for each channel in each cdf 
			for (int c = 0; c < nochannel; c++)
				lastresult[c][i] = std::floor(cdf[c][i] * hightest_intensity);
		}
		for (int y = 0; y < source.GetHeight(); y++) {
			for (int x = 0; x < source.GetWidth(); x++) {
				// look for each channel value
				for (int c = 0; c < nochannel; c++) {
					int index = source(x,y,c);
					equalized(x,y,c) = lastresult[c][index];
				}
				
			
			}
		}

		return equalized;
	}

	Gorgon::Graphics::Bitmap Brightness(const int brightness )const {
		Gorgon::Graphics::Bitmap bmp = source.Duplicate();
		int nochannel = source.GetMode() != Gorgon::Graphics::ColorMode::Grayscale ? 3 : 1;
		int threshhold =brightness>=0? 255 - brightness:0-brightness;
		for (int y = 0; y < source.GetHeight(); y++) {
			for (int x = 0; x < source.GetWidth(); x++) {
				
					for (int c = 0; c < nochannel; c++) {
						if (brightness >= 0) {
							if (source(x, y, c) <= threshhold)
								bmp(x, y, c) = source(x, y, c) + brightness;
							else
								bmp(x, y, c) = 255;
						}
						else {
							if (source(x, y, c) <= threshhold)
								bmp(x, y, c) = 0;
							else
								bmp(x, y, c) = source(x, y, c) + brightness;;
						}
					}
			
			

			}

		}

		return bmp;
	}

	Gorgon::Graphics::Bitmap BrightnessGama(const float gamma) {
		Gorgon::Graphics::Bitmap bmp = source.Duplicate();
		int nochannel = source.GetMode() != Gorgon::Graphics::ColorMode::Grayscale ? 3 : 1;
		for (int y = 0; y < source.GetHeight(); y++) {
			for (int x = 0; x < source.GetWidth(); x++) {

					for (int c = 0; c < nochannel; c++) 
						bmp(x, y, c) = 255 * std::pow(((float)source(x, y, c) / 255), gamma);
					
			
				

			}

		}

		return bmp;


	}
	Gorgon::Graphics::Bitmap Contrast(float c) {

		Gorgon::Graphics::Bitmap bmp = source.Duplicate();
		int nochannel = source.GetMode() != Gorgon::Graphics::ColorMode::Grayscale ? 3 : 1;
		// 255 = 2^b-1 , b is = bit depth our one is =8  so 255 we get max value 
		int max = std::ceil((float)255 / 2);
		//contrast correction  actually parameter correction limited to -255  to 255
		float factor = (259 * (c + 255)) / (255 * (259 - c));
		// contrast correction for limitation value used at here if you dont want you can use below lineand close this. you need to give small value
		//float factor = c;
		for (int y = 0; y < source.GetHeight(); y++) {
			for (int x = 0; x < source.GetWidth(); x++) {

				for (int channel = 0; channel < nochannel; channel++) {
					int value= factor * ((int)source(x, y, channel) - max) + max;
					if (value < 0)
						value = 0;
					else if (value > 255)
						value = 255;

					bmp(x, y, channel) = value;
				}



			}

		}

		return bmp;


	}
	Gorgon::Graphics::Bitmap Invert() {
		Gorgon::Graphics::Bitmap bmp = source.Duplicate();
		int nochannel = source.GetMode() != Gorgon::Graphics::ColorMode::Grayscale ? 3 : 1;
		for (int y = 0; y < source.GetHeight(); y++) {
			for (int x = 0; x < source.GetWidth(); x++) {

				for (int c = 0; c < nochannel; c++)
					bmp(x, y, c) = 255 - source(x, y, c);




			}

		}

		return bmp;


	}
private:
	Gorgon::Graphics::Bitmap source;
	int min[3];
	int max[3];
	// we have 256 different value if our bitdepth = 8  also we have 3 channel 
	int histogram[3][256] = { {} };
	float pdf[3][256] = { {} };
	float cdf[3][256] = { {} };
};
int main(){

	Gorgon::Graphics::Bitmap source;
	source.Import("read.png");
	Histogram histogramprocess(source);
	
	Gorgon::Graphics::Bitmap contraststreched = histogramprocess.ContastStrechingColorFullImage();
	contraststreched.ExportPNG("contrastreching.png");
	std::cout << "Contrast Streching Completeed !" << std::endl;

	Gorgon::Graphics::Bitmap histogramequalization = histogramprocess.HistogramEqualization();
	histogramequalization.ExportPNG("histogramequalization.png");
	std::cout << "Histogram equalization completed ! " << std::endl;
	//brightnes function used  Ixy+b
	Gorgon::Graphics::Bitmap lighter= histogramprocess.Brightness(100);
	lighter.ExportPNG("ligted_brightness.png");
	Gorgon::Graphics::Bitmap darker = histogramprocess.Brightness(-100);
	darker.ExportPNG("darked_brightness.png");
	// gamma funtion
	Gorgon::Graphics::Bitmap Brightnessgamma = histogramprocess.BrightnessGama(2.5);
	Brightnessgamma.ExportPNG("gamma brigtness.png");
	//contrast functions  for negative and positive contrast
	Gorgon::Graphics::Bitmap contrast = histogramprocess.Contrast(-50);
	contrast.ExportPNG("contrastfunction negative.png");
	Gorgon::Graphics::Bitmap contrast2 = histogramprocess.Contrast(50);
	contrast2.ExportPNG("contrastfunction positive.png");
	//invert
	Gorgon::Graphics::Bitmap invert = histogramprocess.Invert();
	invert.ExportPNG("Invert.png");



	std::cout << "Completed !!! " << std::endl;

	

	
	return 0;
}