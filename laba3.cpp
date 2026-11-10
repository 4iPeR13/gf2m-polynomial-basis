#include <iostream>
#include <string>
#include <chrono>
#include <cstdlib>

const int FIELD_SIZE = 173;
const int POLYNOMIAL_DEGREES[5] = { 0, 1, 2, 10, 173 };

class ElementF {
public:
    uint64_t coefficient[FIELD_SIZE / 64 + 1]{};

    ElementF() {}

    ElementF(const ElementF& other) {
        for (int i = 0; i <= FIELD_SIZE / 64; i++) {
            coefficient[i] = other.coefficient[i];
        }
    }

    ElementF(std::string binary) {
        ElementF temp = ConvertToNumber(binary);
        for (int i = 0; i <= FIELD_SIZE / 64; i++) {
            coefficient[i] = temp.coefficient[i];
        }
    }

    ElementF operator+(const ElementF& other) {
        ElementF result;
        for (int i = 0; i <= FIELD_SIZE / 64; i++) {
            result.coefficient[i] = coefficient[i] ^ other.coefficient[i];
        }
        return result;
    }

    ElementF operator*(const ElementF& other) {
        uint64_t result[(2 * FIELD_SIZE - 1) / 64 + 1]{};
        for (int i = 0; i < FIELD_SIZE; i++) {
            for (int j = 0; j < FIELD_SIZE; j++) {
                result[(i + j) / 64] ^= (((coefficient[i / 64] >> (i % 64)) & (other.coefficient[j / 64] >> (j % 64))) & 1ULL) << ((i + j) % 64);
            }
        }
        return ReducePolynomial(result);
    }

    bool operator==(const ElementF& other) {
        for (int i = 0; i <= FIELD_SIZE / 64; i++) {
            if (coefficient[i] != other.coefficient[i]) return false;
        }
        return true;
    }

    ElementF square() {
        uint64_t result[(2 * FIELD_SIZE - 1) / 64 + 1]{};
        for (int i = 0; i < FIELD_SIZE; i++) {
            uint64_t bit = (coefficient[i / 64] >> (i % 64)) & 1;
            result[2 * i / 64] |= bit << (2 * i % 64);
        }
        return ReducePolynomial(result);
    }

    ElementF power(ElementF& other) {
        ElementF result = one();
        for (int i = FIELD_SIZE - 1; i >= 0; i--) {
            uint64_t bit = (other.coefficient[i / 64] >> (i % 64)) & 1;
            if (bit == 1) result = *this * result;
            if (i > 0) result = result.square();
        }
        return result;
    }

    ElementF trace() {
        ElementF temp(*this);
        ElementF result(*this);
        for (int i = 1; i < FIELD_SIZE; i++) {
            temp = temp.square();
            result = result + temp;
        }
        return result;
    }

    ElementF inv() {
        ElementF temp(*this);
        ElementF result = one();
        for (int i = 0; i < FIELD_SIZE - 1; i++) {
            temp = temp.square();
            result = result * temp;
        }
        return result;
    }

    friend std::ostream& operator<<(std::ostream& os, const ElementF& element) {
        os << ConvertToBinary(element);
        return os;
    }

private:
    static int PolynomialDegree(uint64_t poly[]) {
        for (int i = (2 * FIELD_SIZE - 1); i >= 0; i--) {
            if ((poly[i / 64] >> (i % 64)) & 1) return i;
        }
        return 0;
    }

    static ElementF ReducePolynomial(uint64_t poly[]) {
        int d = PolynomialDegree(poly);
        while (d >= FIELD_SIZE) {
            for (int i : POLYNOMIAL_DEGREES) {
                int index = d + i - FIELD_SIZE;
                poly[index / 64] ^= 1ULL << (index % 64);
            }
            d = PolynomialDegree(poly);
        }
        ElementF result;
        for (int i = 0; i <= FIELD_SIZE / 64; i++) {
            result.coefficient[i] = poly[i];
        }
        return result;
    }

    static ElementF ConvertToNumber(std::string binary) {
        ElementF result;
        for (int i = binary.length() - 1; i >= 0; i--) {
            if (binary[binary.length() - 1 - i] == '1') {
                result.coefficient[i / 64] |= 1ULL << (i % 64);
            }
        }
        return result;
    }

    static std::string ConvertToBinary(const ElementF& element) {
        std::string binary;
        for (int i = FIELD_SIZE - 1; i >= 0; i--) {
            binary += ((element.coefficient[i / 64] >> (i % 64)) & 1) ? '1' : '0';
        }
        return binary;
    }

    static ElementF& one() {
        static ElementF result("1");
        return result;
    }

    static ElementF& zero() {
        static ElementF result;
        return result;
    }
};

ElementF generateElementF() {
    std::string binary = "01";
    std::string result = "";
    for (int i = 0; i < FIELD_SIZE; i++) {
        result += binary[rand() % 2];
    }
    return ElementF(result);
}

void measureTime(int numberOfMeasurements) {
    long long nanoseconds = 0;
    for (int i = 0; i < numberOfMeasurements; i++) {
        ElementF first = generateElementF();
        ElementF second = generateElementF();

        auto start = std::chrono::high_resolution_clock::now();
        first + second;
        auto end = std::chrono::high_resolution_clock::now();
        nanoseconds += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    }
    std::cout << "Elapsed nanoseconds for addition: " << nanoseconds / numberOfMeasurements << '\n';
    nanoseconds = 0;

    for (int i = 0; i < numberOfMeasurements; i++) {
        ElementF first = generateElementF();
        ElementF second = generateElementF();

        auto start = std::chrono::high_resolution_clock::now();
        first* second;
        auto end = std::chrono::high_resolution_clock::now();
        nanoseconds += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    }
    std::cout << "Elapsed nanoseconds for multiplication: " << nanoseconds / numberOfMeasurements << '\n';
    nanoseconds = 0;

    for (int i = 0; i < numberOfMeasurements; i++) {
        ElementF first = generateElementF();

        auto start = std::chrono::high_resolution_clock::now();
        first.square();
        auto end = std::chrono::high_resolution_clock::now();
        nanoseconds += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    }
    std::cout << "Elapsed nanoseconds for square: " << nanoseconds / numberOfMeasurements << '\n';
    nanoseconds = 0;

    for (int i = 0; i < numberOfMeasurements; i++) {
        ElementF first = generateElementF();
        ElementF second = generateElementF();

        auto start = std::chrono::high_resolution_clock::now();
        first.power(second);
        auto end = std::chrono::high_resolution_clock::now();
        nanoseconds += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    }
    std::cout << "Elapsed nanoseconds for power: " << nanoseconds / numberOfMeasurements << '\n';
    nanoseconds = 0;

    for (int i = 0; i < numberOfMeasurements; i++) {
        ElementF first = generateElementF();

        auto start = std::chrono::high_resolution_clock::now();
        first.inv();
        auto end = std::chrono::high_resolution_clock::now();
        nanoseconds += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    }
    std::cout << "Elapsed nanoseconds for inverse: " << nanoseconds / numberOfMeasurements << '\n';
    nanoseconds = 0;

    for (int i = 0; i < numberOfMeasurements; i++) {
        ElementF first = generateElementF();

        auto start = std::chrono::high_resolution_clock::now();
        first.trace();
        auto end = std::chrono::high_resolution_clock::now();
        nanoseconds += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    }
    std::cout << "Elapsed nanoseconds for trace: " << nanoseconds / numberOfMeasurements << '\n';
}

int main() {
    ElementF f("10111100011111111000001110001001100100010000011110000110101100001111100001000110000011110110001001111011101110110110111010001111110000010101010001110001110011011011001111111");
    ElementF g("10000000100101000011001011000001000000000101100011110011110001001011100011011111110101001011011110000010101111110101100010110101001010000110110101000111111100010011010001101");
    ElementF h("01001111000101101100010001100011100011100101010000011110011011010001000000001000110100100101110010010110101010110111011100011111100000100001110111000100100011010010101100101");

    std::cout << "A = " << f << "\n\n";
    std::cout << "B = " << g << "\n\n";
    std::cout << "N = " << h << "\n\n";

    std::cout << "A + B = " << (f + g) << "\n\n";
    std::cout << "A * B = " << (f * g) << "\n\n";
    std::cout << "A^2 = " << f.square() << "\n\n";
    std::cout << "A^N = " << f.power(h) << "\n\n";
    std::cout << "A^-1 = " << f.inv() << "\n\n";
    std::cout << "Tr(A) = " << f.trace() << "\n\n";

    measureTime(200);

    return 0;
}
