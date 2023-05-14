template <typename T>
class Float {
public:
    explicit Float(T value) : value_(value) {}

    operator T() const { return value_; }

private:
    T value_;
};

int main () {
    Float<double> x(3.0f);
    return 0;
}

