#define _USE_MATH_DEFINES

#include <SFML/Graphics.hpp>
#include <cmath>

using namespace std;
using namespace sf;

struct Matrix {
	int rows = 0, cols = 0;
	vector<double> values;

	Matrix(const int& cols, const int& rows, const vector<double>& values) {
		this->rows = rows;
		this->cols = cols;
		this->values = values;
	}

	static Matrix vectorXY(const Vector2f& vec) {
		return Matrix(3, 1, { vec.x, vec.y, 1 });
	}

	double getValue(const int& i, const int& j) const {
		return values[rows * i + j];
	}

	Vector2f toVector2f() const {
		return Vector2f((float)getValue(0, 0), (float)getValue(1, 0));
	}

	Matrix operator*(const Matrix& a) const {
		const Matrix& b = *this;
		int rows = a.rows, cols = b.cols;
		vector<double> v;
		double sum = 0;

		for (int i = 0; i < rows; i++)
			for (int j = 0; j < cols; j++) {
				sum = 0;
				for (int k = 0; k < a.cols; k++)
					sum += a.getValue(i, k) * b.getValue(k, j);
				v.push_back(sum);
			}

		return Matrix(rows, cols, v);
	}

	static Matrix Translation(const double& x, const double& y) {
		return Matrix(3, 3, { 1, 0, x,
							  0, 1, y,
							  0, 0, 1 });
	}

	static Matrix RotationAroundOrigin(const double& phi) {
		return Matrix(3, 3, { cos(phi), sin(phi), 0,
							 -sin(phi), cos(phi), 0,
								 0,        0,     1 });
	}

	static Matrix ScaleOrigin(const double& x, const double& y) {
		return Matrix(3, 3, { x, 0, 0,
							  0, y, 0,
							  0, 0, 1 });
	}

	static Matrix ShearHOrigin(const double& m) {
		return Matrix(3, 3, { 1, m, 0,
							  0, 1, 0,
							  0, 0, 1 });
	}

	static Matrix ShearVOrigin(const double& m) {
		return Matrix(3, 3, { 1, 0, 0,
							  m, 1, 0,
							  0, 0, 1 });
	}
};

struct PolygonShape {
	VertexArray vertices;

	PolygonShape(const int& n) {
		srand(unsigned(time(NULL)));
		vertices.setPrimitiveType(LineStrip);
		vertices.clear();
		for (int i = 0; i < n; i++) {
			vertices.append(Vertex(Vector2f((float)(rand() % 1000), (float)(rand() % 1000)), Color::White));
		}
		vertices.append(vertices[0]);
	}

	PolygonShape(const PolygonShape& p) {
		vertices.setPrimitiveType(LineStrip);
		vertices.clear();
		for (int i = 0; i < p.vertices.getVertexCount(); i++) {
			vertices.append(p.vertices[i]);
		}
	}

	void Transform(const Matrix& a) {
		for (int i = 0; i < vertices.getVertexCount(); i++) {
			vertices[i].position = (Matrix::vectorXY(vertices[i].position) * a).toVector2f();
		}
	}

	void Translate(const double& x, const double& y) {
		Transform(Matrix::Translation(x, y));
	}

	void RotateAroundOrigin(const double& phi) {
		Transform(Matrix::RotationAroundOrigin(phi));
	}

	void RotateAroundPoint(const double& x, const double& y, const double& phi) {
		Translate(-x, -y);
		RotateAroundOrigin(phi);
		Translate(x, y);
	}

	void ScaleAroundOrigin(const double& kx, const double& ky) {
		Transform(Matrix::ScaleOrigin(kx, ky));
	}

	void scaleAroundPoint(const double& x, const double& y, const double& kx, const double& ky) {
		Translate(-x, -y);
		ScaleAroundOrigin(kx, ky);
		Translate(x, y);
	}

	void ShearAroundOrigin(const double& my) {
		Transform(Matrix::ShearVOrigin(my));
	}

	void ShearAroundPoint(const double& x, const double& y, const double& my) {
		Translate(-x, -y);
		ShearAroundOrigin(my);
		Translate(x, y);
	}

	void ShearAroundPointWithAxis(const double& x, const double& y, const double& phi, const double& my) {
		Translate(-x, -y);
		RotateAroundOrigin(phi);
		ShearAroundOrigin(my);
		RotateAroundOrigin(-phi);
		Translate(x, y);
	}

	Vector2f getMassCenter() const {
		float sumX = 0, sumY = 0;
		int vc = int(vertices.getVertexCount());

		for (int i = 0; i < vc; i++) {
			sumX += vertices[i].position.x;
			sumY += vertices[i].position.y;
		}

		return Vector2f(sumX / vc, sumY / vc);
	}
};

int main() {
	int a;
	PolygonShape original_shape(4), shape = original_shape;
	RenderWindow window(VideoMode(1000, 1000), "Lab 3");
	Event event;
	Clock clock;
	int animation = 0;

	while (window.isOpen()) {
		Vector2f massCenter = shape.getMassCenter();

		while (window.pollEvent(event)) {
			if (event.type == Event::Closed) window.close();

			if (event.type == Event::KeyPressed) {
				switch (event.key.code) {
				case Keyboard::Left:
					shape.Translate(-10, 0);
					break;

				case Keyboard::Right:
					shape.Translate(10, 0);
					break;

				case Keyboard::Up:
					shape.Translate(0, -10);
					break;

				case Keyboard::Down:
					shape.Translate(0, 10);
					break;

				case Keyboard::Q:
					shape.RotateAroundPoint(massCenter.x, massCenter.y, M_PI_4);
					break;

				case Keyboard::E:
					shape.RotateAroundPoint(massCenter.x, massCenter.y, -M_PI_4);
					break;

				case Keyboard::W:
					shape.scaleAroundPoint(massCenter.x, massCenter.y, 2, 2);
					break;

				case Keyboard::S:
					shape.scaleAroundPoint(massCenter.x, massCenter.y, 0.5, 0.5);
					break;

				case Keyboard::D:
					shape.ShearAroundPointWithAxis(massCenter.x, massCenter.y, M_PI_4 * 3, M_SQRT2);
					break;

				case Keyboard::A:
					shape.ShearAroundPointWithAxis(massCenter.x, massCenter.y, 3 * M_PI_4, -M_SQRT2);
					break;

				case Keyboard::Space:
					animation = (animation + 1) % 5;
					shape = PolygonShape(original_shape);
					break;

				case Keyboard::R:
					shape = PolygonShape(original_shape);
					break;

				default:
					break;
				}
			}
		}

		Time elapsed = clock.restart();
		float dt = elapsed.asSeconds();
		switch (animation) {
		case 1:
			shape.Translate(50.f * dt, -100.f * dt);
			break;

		case 2:
			shape.RotateAroundPoint(massCenter.x, massCenter.y, M_PI / 10. * dt);
			break;

		case 3:
			shape.scaleAroundPoint(massCenter.x, massCenter.y, dt / 10.f + 1, dt + 1);
			break;

		case 4:
			shape.ShearAroundPointWithAxis(massCenter.x, massCenter.y, 3 * M_PI_4, M_SQRT2 * dt);
			break;

		default:
			break;
		}

		window.clear();
		window.draw(shape.vertices);
		window.display();
	}
	return 0;
}
