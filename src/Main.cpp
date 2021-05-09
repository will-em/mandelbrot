#include "Platform/Platform.hpp"
#include "cmath"
#include <chrono>
using namespace std::chrono;

#define height 600
#define width 900

void generate_image(sf::Image& image, std::vector<double> interval, int max_iterations);

void generate_image(sf::Image& image, std::vector<double> interval, int max_iterations)
{
	double h_x = (interval.at(1) - interval.at(0)) / (1.0 * width);
	double h_y = (interval.at(3) - interval.at(2)) / (1.0 * height);
	image.create(width, height);
	double x0 = interval.at(0);
	double y0 = interval.at(2);
	double x, y, x2, y2;
	bool symmetry;

	std::array<std::array<sf::Uint8, height>, width> n_arr;
	auto start = high_resolution_clock::now();
	for (int i = 0; i < height; i++)
	{
		x0 = interval.at(0);

		symmetry = y0 > 0.0 && -y0 >= interval.at(2);

		for (int j = 0; j < width; j++)
		{
			if (symmetry)
			{
				int index = (int)((-y0 - interval.at(2)) / h_y);
				n_arr[j][i] = n_arr[j][index];
				continue;
			}
			x = 0.0;
			y = 0.0;
			x2 = 0.0;
			y2 = 0.0;
			sf::Uint8 n = 0;
			while (x * x + y * y <= 4 && n < max_iterations)
			{
				y = 2 * x * y + y0;
				x = x2 - y2 + x0;
				x2 = x * x;
				y2 = y * y;
				n += 1;
			}

			n_arr[j][i] = n;

			x0 += h_x;
		}
		y0 += h_y;
	}
	auto stop = high_resolution_clock::now();
	auto duration = duration_cast<microseconds>(stop - start);
	std::cout << "First loop " << duration.count() << std::endl;
	start = high_resolution_clock::now();
	sf::Uint8 n;
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			n = n_arr[j][i];
			image.setPixel(j, i, sf::Color(255 * n / max_iterations, 50 * n / max_iterations, 50 * n / max_iterations));
		}
	}
	stop = high_resolution_clock::now();
	duration = duration_cast<microseconds>(stop - start);

	std::cout << "Second loop " << duration.count() << std::endl;
}

int main()
{
	double zoom = 1.05;
	double sensitivity = 1.0;
	std::vector<double> interval = { -2, 1, -1, 1 }; //x_start, x_end, y_start, y_end

	int max_iterations = 100;

	util::Platform platform;

	sf::RenderWindow window;
	// in Windows at least, this must be called before creating the window
	float screenScalingFactor = platform.getScreenScalingFactor(window.getSystemHandle());
	// Use the screenScalingFactor
	window.create(sf::VideoMode(width * screenScalingFactor, height * screenScalingFactor), "SFML works!");
	platform.setIcon(window.getSystemHandle());
	sf::Image image;
	generate_image(image, interval, max_iterations);

	sf::Texture texture;
	texture.loadFromImage(image);

	sf::Sprite sprite(texture);

	sf::Event event;
	window.clear();
	bool buttonHold = false;
	int init_x = 0;
	int init_y = 0;
	while (window.isOpen())
	{
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
			if (event.type == sf::Event::MouseButtonPressed)
			{
				if (event.mouseButton.button == sf::Mouse::Left)
				{
					buttonHold = true;
					init_x = event.mouseButton.x;
					init_y = event.mouseButton.y;
				}
			}
			else if (event.type == sf::Event::MouseButtonReleased)
			{
				if (event.mouseButton.button == sf::Mouse::Left)
				{
					buttonHold = false;
				}
			}
			if (event.type == sf::Event::MouseMoved)
			{
				if (buttonHold)
				{
					if (event.mouseMove.x != init_x || event.mouseMove.y != init_y) //Panning
					{
						double delta_x = event.mouseMove.x - init_x;
						double delta_y = event.mouseMove.y - init_y;
						interval.at(0) -= sensitivity * (interval.at(1) - interval.at(0)) * delta_x / (2.0 * width);
						interval.at(1) -= sensitivity * (interval.at(1) - interval.at(0)) * delta_x / (2.0 * width);
						interval.at(2) -= sensitivity * (interval.at(3) - interval.at(2)) * delta_y / (2.0 * height);
						interval.at(3) -= sensitivity * (interval.at(3) - interval.at(2)) * delta_y / (2.0 * height);

						init_x = event.mouseMove.x;
						init_y = event.mouseMove.y;
						generate_image(image, interval, max_iterations);
						texture.loadFromImage(image);
						sf::Sprite sprite(texture);
					}
				}
			}

			if (event.type == sf::Event::MouseWheelMoved)
			{
				double zoom_factor = (event.mouseWheelScroll.delta > 0) ? (zoom) : (1.0 / zoom);
				double mouse_x_before = interval.at(0) + (interval.at(1) - interval.at(0)) * event.mouseWheel.x / (1.0 * width);
				double mouse_y_before = interval.at(2) + (interval.at(3) - interval.at(2)) * event.mouseWheel.y / (1.0 * height);

				//Zoom
				interval.at(0) /= zoom_factor;
				interval.at(1) /= zoom_factor;
				interval.at(2) /= zoom_factor;
				interval.at(3) /= zoom_factor;

				double mouse_x_after = interval.at(0) + (interval.at(1) - interval.at(0)) * event.mouseWheel.x / (1.0 * width);
				double mouse_y_after = interval.at(2) + (interval.at(3) - interval.at(2)) * event.mouseWheel.y / (1.0 * height);

				//Correction
				interval.at(0) -= mouse_x_after - mouse_x_before;
				interval.at(1) -= mouse_x_after - mouse_x_before;
				interval.at(2) -= mouse_y_after - mouse_y_before;
				interval.at(3) -= mouse_y_after - mouse_y_before;

				generate_image(image, interval, max_iterations);
				texture.loadFromImage(image);
				sf::Sprite sprite(texture);
			}
		}
		window.draw(sprite);
		window.display();
	}

	return 0;
}
