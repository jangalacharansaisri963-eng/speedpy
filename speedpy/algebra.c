#include <Python.h>
#include <math.h>

static double sp_alg_abs(double x)
{
    return x < 0.0 ? -x : x;
}

static double sp_alg_sign(double x)
{
    if (x > 0.0)
        return 1.0;
    if (x < 0.0)
        return -1.0;
    return 0.0;
}

static double sp_alg_mod(double a, double b)
{
    return fmod(a, b);
}

static double sp_alg_floor(double x)
{
    return floor(x);
}

static double sp_alg_ceil(double x)
{
    return ceil(x);
}

static double sp_alg_square(double x)
{
    return x * x;
}

static double sp_alg_cube(double x)
{
    return x * x * x;
}

static double sp_alg_power(double a, double b)
{
    return pow(a, b);
}

static PyObject *sp_alg_add(PyObject *self, PyObject *args)
{
    double a, b;

    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    return PyFloat_FromDouble(a + b);
}

static PyObject *sp_alg_subtract(PyObject *self, PyObject *args)
{
    double a, b;

    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    return PyFloat_FromDouble(a - b);
}

static PyObject *sp_alg_multiply(PyObject *self, PyObject *args)
{
    double a, b;

    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    return PyFloat_FromDouble(a * b);
}

static PyObject *sp_alg_divide(PyObject *self, PyObject *args)
{
    double a, b;

    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    if (b == 0.0)
    {
        PyErr_SetString(PyExc_ZeroDivisionError, "division by zero");
        return NULL;
    }

    return PyFloat_FromDouble(a / b);
}

static PyObject *sp_alg_abs(PyObject *self, PyObject *args)
{
    double x;

    if (!PyArg_ParseTuple(args, "d", &x))
        return NULL;

    return PyFloat_FromDouble(sp_alg_abs(x));
}

static PyObject *sp_alg_sign(PyObject *self, PyObject *args)
{
    double x;

    if (!PyArg_ParseTuple(args, "d", &x))
        return NULL;

    return PyFloat_FromDouble(sp_alg_sign(x));
}

static PyObject *sp_alg_mod(PyObject *self, PyObject *args)
{
    double a, b;

    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    if (b == 0.0)
    {
        PyErr_SetString(PyExc_ZeroDivisionError, "modulo by zero");
        return NULL;
    }

    return PyFloat_FromDouble(sp_alg_mod(a, b));
}

static PyObject *sp_alg_floor(PyObject *self, PyObject *args)
{
    double x;

    if (!PyArg_ParseTuple(args, "d", &x))
        return NULL;

    return PyFloat_FromDouble(sp_alg_floor(x));
}

static PyObject *sp_alg_ceil(PyObject *self, PyObject *args)
{
    double x;

    if (!PyArg_ParseTuple(args, "d", &x))
        return NULL;

    return PyFloat_FromDouble(sp_alg_ceil(x));
}

static PyObject *sp_alg_square(PyObject *self, PyObject *args)
{
    double x;

    if (!PyArg_ParseTuple(args, "d", &x))
        return NULL;

    return PyFloat_FromDouble(sp_alg_square(x));
}

static PyObject *sp_alg_cube(PyObject *self, PyObject *args)
{
    double x;

    if (!PyArg_ParseTuple(args, "d", &x))
        return NULL;

    return PyFloat_FromDouble(sp_alg_cube(x));
}

static PyObject *sp_alg_power(PyObject *self, PyObject *args)
{
    double a, b;

    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    return PyFloat_FromDouble(sp_alg_power(a, b));
}

static PyObject *sp_alg_negate(PyObject *self, PyObject *args)
{
    double x;

    if (!PyArg_ParseTuple(args, "d", &x))
        return NULL;

    return PyFloat_FromDouble(-x);
}

static PyObject *sp_alg_reciprocal(PyObject *self, PyObject *args)
{
    double x;

    if (!PyArg_ParseTuple(args, "d", &x))
        return NULL;

    if (x == 0.0)
    {
        PyErr_SetString(PyExc_ZeroDivisionError, "reciprocal of zero");
        return NULL;
    }

    return PyFloat_FromDouble(1.0 / x);
}

static PyObject *sp_alg_double(PyObject *self, PyObject *args)
{
    double x;

    if (!PyArg_ParseTuple(args, "d", &x))
        return NULL;

    return PyFloat_FromDouble(2.0 * x);
}

static PyObject *sp_alg_half(PyObject *self, PyObject *args)
{
    double x;

    if (!PyArg_ParseTuple(args, "d", &x))
        return NULL;

    return PyFloat_FromDouble(x / 2.0);
}

static PyObject *sp_alg_triple(PyObject *self, PyObject *args)
{
    double x;

    if (!PyArg_ParseTuple(args, "d", &x))
        return NULL;

    return PyFloat_FromDouble(3.0 * x);
}

static PyObject *sp_alg_quarter(PyObject *self, PyObject *args)
{
    double x;

    if (!PyArg_ParseTuple(args, "d", &x))
        return NULL;

    return PyFloat_FromDouble(x / 4.0);
}

static PyObject *sp_alg_increment(PyObject *self, PyObject *args)
{
    double x;

    if (!PyArg_ParseTuple(args, "d", &x))
        return NULL;

    return PyFloat_FromDouble(x + 1.0);
}

static PyObject *sp_alg_decrement(PyObject *self, PyObject *args)
{
    double x;

    if (!PyArg_ParseTuple(args, "d", &x))
        return NULL;

    return PyFloat_FromDouble(x - 1.0);
}

static PyObject *sp_alg_average_two(PyObject *self, PyObject *args)
{
    double a, b;

    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    return PyFloat_FromDouble((a + b) / 2.0);
}

static PyObject *sp_alg_average_three(PyObject *self, PyObject *args)
{
    double a, b, c;

    if (!PyArg_ParseTuple(args, "ddd", &a, &b, &c))
        return NULL;

    return PyFloat_FromDouble((a + b + c) / 3.0);
}

static PyObject *sp_alg_linear_value(PyObject *self, PyObject *args)
{
    double a, x, b;

    if (!PyArg_ParseTuple(args, "ddd", &a, &x, &b))
        return NULL;

    return PyFloat_FromDouble(a * x + b);
}

static PyObject *sp_alg_linear_slope(PyObject *self, PyObject *args)
{
    double x1, y1, x2, y2;

    if (!PyArg_ParseTuple(args, "dddd", &x1, &y1, &x2, &y2))
        return NULL;

    if (x2 == x1)
    {
        PyErr_SetString(PyExc_ZeroDivisionError, "vertical line has no finite slope");
        return NULL;
    }

    return PyFloat_FromDouble((y2 - y1) / (x2 - x1));
}

static PyObject *sp_alg_linear_intercept(PyObject *self, PyObject *args)
{
    double a, x, y;

    if (!PyArg_ParseTuple(args, "ddd", &a, &x, &y))
        return NULL;

    return PyFloat_FromDouble(y - a * x);
}

static PyObject *sp_alg_distance_on_number_line(PyObject *self, PyObject *args)
{
    double a, b;

    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    return PyFloat_FromDouble(sp_alg_abs(a - b));
}

static PyObject *sp_alg_midpoint(PyObject *self, PyObject *args)
{
    double a, b;

    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    return PyFloat_FromDouble((a + b) / 2.0);
}

static PyObject *sp_alg_weighted_average(PyObject *self, PyObject *args)
{
    double a, b, wa, wb;

    if (!PyArg_ParseTuple(args, "dddd", &a, &b, &wa, &wb))
        return NULL;

    if (wa + wb == 0.0)
    {
        PyErr_SetString(PyExc_ZeroDivisionError, "sum of weights cannot be zero");
        return NULL;
    }

    return PyFloat_FromDouble((a * wa + b * wb) / (wa + wb));
}

static PyObject *sp_alg_percentage(PyObject *self, PyObject *args)
{
    double value, total;

    if (!PyArg_ParseTuple(args, "dd", &value, &total))
        return NULL;

    if (total == 0.0)
    {
        PyErr_SetString(PyExc_ZeroDivisionError, "total cannot be zero");
        return NULL;
    }

    return PyFloat_FromDouble((value / total) * 100.0);
}

static PyObject *sp_alg_percent_of(PyObject *self, PyObject *args)
{
    double percent, value;

    if (!PyArg_ParseTuple(args, "dd", &percent, &value))
        return NULL;

    return PyFloat_FromDouble((percent / 100.0) * value);
}

static PyObject *sp_alg_ratio(PyObject *self, PyObject *args)
{
    double a, b;

    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    if (b == 0.0)
    {
        PyErr_SetString(PyExc_ZeroDivisionError, "ratio denominator cannot be zero");
        return NULL;
    }

    return PyFloat_FromDouble(a / b);
}

static PyObject *sp_alg_proportion(PyObject *self, PyObject *args)
{
    double a, b, c;

    if (!PyArg_ParseTuple(args, "ddd", &a, &b, &c))
        return NULL;

    if (b == 0.0)
    {
        PyErr_SetString(PyExc_ZeroDivisionError, "proportion denominator cannot be zero");
        return NULL;
    }

    return PyFloat_FromDouble((a / b) * c);
}

static PyObject *sp_alg_sum_to_n(PyObject *self, PyObject *args)
{
    double n;

    if (!PyArg_ParseTuple(args, "d", &n))
        return NULL;

    return PyFloat_FromDouble(n * (n + 1.0) / 2.0);
}

static PyObject *sp_alg_sum_squares_to_n(PyObject *self, PyObject *args)
{
    double n;

    if (!PyArg_ParseTuple(args, "d", &n))
        return NULL;

    return PyFloat_FromDouble(n * (n + 1.0) * (2.0 * n + 1.0) / 6.0);
}

static PyObject *sp_alg_sum_cubes_to_n(PyObject *self, PyObject *args)
{
    double n;
    double s;

    if (!PyArg_ParseTuple(args, "d", &n))
        return NULL;

    s = n * (n + 1.0) / 2.0;

    return PyFloat_FromDouble(sp_alg_square(s));
}

static PyObject *sp_alg_square_difference(PyObject *self, PyObject *args)
{
    double a, b;

    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    return PyFloat_FromDouble(sp_alg_square(a) - sp_alg_square(b));
}

static PyObject *sp_alg_cube_difference(PyObject *self, PyObject *args)
{
    double a, b;

    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    return PyFloat_FromDouble(sp_alg_cube(a) - sp_alg_cube(b));
}

static PyObject *sp_alg_square_sum(PyObject *self, PyObject *args)
{
    double a, b;

    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_square(a) +
        2.0 * a * b +
        sp_alg_square(b)
    );
}

static PyObject *sp_alg_cube_sum(PyObject *self, PyObject *args)
{
    double a, b;

    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_cube(a) +
        3.0 * sp_alg_square(a) * b +
        3.0 * a * sp_alg_square(b) +
        sp_alg_cube(b)
    );
}

static PyObject *sp_alg_difference_of_cubes(PyObject *self, PyObject *args)
{
    double a, b;

    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_cube(a) -
        3.0 * sp_alg_square(a) * b +
        3.0 * a * sp_alg_square(b) -
        sp_alg_cube(b)
    );
}

static PyObject *sp_alg_square_sum_value(PyObject *self, PyObject *args)
{
    double a, b;

    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    return PyFloat_FromDouble(sp_alg_square(a + b));
}

static PyObject *sp_alg_square_difference_value(PyObject *self, PyObject *args)
{
    double a, b;

    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    return PyFloat_FromDouble(sp_alg_square(a - b));
}

static PyObject *sp_alg_cube_sum_value(PyObject *self, PyObject *args)
{
    double a, b;

    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    return PyFloat_FromDouble(sp_alg_cube(a + b));
}

static PyObject *sp_alg_cube_difference_value(PyObject *self, PyObject *args)
{
    double a, b;

    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    return PyFloat_FromDouble(sp_alg_cube(a - b));
}

static PyObject *sp_alg_product_sum(PyObject *self, PyObject *args)
{
    double a, b, c;

    if (!PyArg_ParseTuple(args, "ddd", &a, &b, &c))
        return NULL;

    return PyFloat_FromDouble(a * b + c);
}

static PyObject *sp_alg_product_difference(PyObject *self, PyObject *args)
{
    double a, b, c;

    if (!PyArg_ParseTuple(args, "ddd", &a, &b, &c))
        return NULL;

    return PyFloat_FromDouble(a * b - c);
}

static PyObject *sp_alg_linear_combination(PyObject *self, PyObject *args)
{
    double a, b, x, y;

    if (!PyArg_ParseTuple(args, "dddd", &a, &b, &x, &y))
        return NULL;

    return PyFloat_FromDouble(a * x + b * y);
}

static PyObject *sp_alg_difference_linear_terms(PyObject *self, PyObject *args)
{
    double a, x, b, y;

    if (!PyArg_ParseTuple(args, "dddd", &a, &x, &b, &y))
        return NULL;

    return PyFloat_FromDouble(a * x - b * y);
}

static PyObject *sp_alg_sum_three(PyObject *self, PyObject *args)
{
    double a, b, c;

    if (!PyArg_ParseTuple(args, "ddd", &a, &b, &c))
        return NULL;

    return PyFloat_FromDouble(a + b + c);
}

static PyObject *sp_alg_product_three(PyObject *self, PyObject *args)
{
    double a, b, c;

    if (!PyArg_ParseTuple(args, "ddd", &a, &b, &c))
        return NULL;

    return PyFloat_FromDouble(a * b * c);
}

static PyObject *sp_alg_sum_four(PyObject *self, PyObject *args)
{
    double a, b, c, d;

    if (!PyArg_ParseTuple(args, "dddd", &a, &b, &c, &d))
        return NULL;

    return PyFloat_FromDouble(a + b + c + d);
}

static PyObject *sp_alg_product_four(PyObject *self, PyObject *args)
{
    double a, b, c, d;

    if (!PyArg_ParseTuple(args, "dddd", &a, &b, &c, &d))
        return NULL;

    return PyFloat_FromDouble(a * b * c * d);
}

static PyObject *sp_alg_mean_three(PyObject *self, PyObject *args)
{
    double a, b, c;

    if (!PyArg_ParseTuple(args, "ddd", &a, &b, &c))
        return NULL;

    return PyFloat_FromDouble((a + b + c) / 3.0);
}

static PyObject *sp_alg_mean_four(PyObject *self, PyObject *args)
{
    double a, b, c, d;

    if (!PyArg_ParseTuple(args, "dddd", &a, &b, &c, &d))
        return NULL;

    return PyFloat_FromDouble((a + b + c + d) / 4.0);
}

static PyObject *sp_alg_range(PyObject *self, PyObject *args)
{
    double a, b;

    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    return PyFloat_FromDouble(sp_alg_abs(b - a));
}

static PyObject *sp_alg_absolute_difference(PyObject *self, PyObject *args)
{
    double a, b;

    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    return PyFloat_FromDouble(sp_alg_abs(a - b));
}

static PyObject *sp_alg_relative_difference(PyObject *self, PyObject *args)
{
    double a, b, denominator;

    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    denominator = sp_alg_abs(b);

    if (denominator == 0.0)
    {
        PyErr_SetString(PyExc_ZeroDivisionError, "reference value cannot be zero");
        return NULL;
    }

    return PyFloat_FromDouble(sp_alg_abs(a - b) / denominator);
}

static PyObject *sp_alg_relative_error(PyObject *self, PyObject *args)
{
    double approximate, exact;

    if (!PyArg_ParseTuple(args, "dd", &approximate, &exact))
        return NULL;

    if (exact == 0.0)
    {
        PyErr_SetString(PyExc_ZeroDivisionError, "exact value cannot be zero");
        return NULL;
    }

    return PyFloat_FromDouble(
        sp_alg_abs(approximate - exact) / sp_alg_abs(exact)
    );
}

static PyObject *sp_alg_percent_error(PyObject *self, PyObject *args)
{
    double approximate, exact;

    if (!PyArg_ParseTuple(args, "dd", &approximate, &exact))
        return NULL;

    if (exact == 0.0)
    {
        PyErr_SetString(PyExc_ZeroDivisionError, "exact value cannot be zero");
        return NULL;
    }

    return PyFloat_FromDouble(
        (sp_alg_abs(approximate - exact) / sp_alg_abs(exact)) * 100.0
    );
}

static PyObject *sp_alg_percent_change(PyObject *self, PyObject *args)
{
    double old_value, new_value;

    if (!PyArg_ParseTuple(args, "dd", &old_value, &new_value))
        return NULL;

    if (old_value == 0.0)
    {
        PyErr_SetString(PyExc_ZeroDivisionError, "old value cannot be zero");
        return NULL;
    }

    return PyFloat_FromDouble(
        ((new_value - old_value) / sp_alg_abs(old_value)) * 100.0
    );
}

static PyObject *sp_alg_increase_by_percent(PyObject *self, PyObject *args)
{
    double value, percent;

    if (!PyArg_ParseTuple(args, "dd", &value, &percent))
        return NULL;

    return PyFloat_FromDouble(
        value + value * percent / 100.0
    );
}

static PyObject *sp_alg_decrease_by_percent(PyObject *self, PyObject *args)
{
    double value, percent;

    if (!PyArg_ParseTuple(args, "dd", &value, &percent))
        return NULL;

    return PyFloat_FromDouble(
        value - value * percent / 100.0
    );
}

static PyObject *sp_alg_percent_increase(PyObject *self, PyObject *args)
{
    double value, percent;

    if (!PyArg_ParseTuple(args, "dd", &value, &percent))
        return NULL;

    return PyFloat_FromDouble(
        value * (1.0 + percent / 100.0)
    );
}

static PyObject *sp_alg_percent_decrease(PyObject *self, PyObject *args)
{
    double value, percent;

    if (!PyArg_ParseTuple(args, "dd", &value, &percent))
        return NULL;

    return PyFloat_FromDouble(
        value * (1.0 - percent / 100.0)
    );
}

static PyObject *sp_alg_scale(PyObject *self, PyObject *args)
{
    double value, factor;

    if (!PyArg_ParseTuple(args, "dd", &value, &factor))
        return NULL;

    return PyFloat_FromDouble(value * factor);
}

static PyObject *sp_alg_unscale(PyObject *self, PyObject *args)
{
    double value, factor;

    if (!PyArg_ParseTuple(args, "dd", &value, &factor))
        return NULL;

    if (factor == 0.0)
    {
        PyErr_SetString(PyExc_ZeroDivisionError, "scale factor cannot be zero");
        return NULL;
    }

    return PyFloat_FromDouble(value / factor);
}

static PyObject *sp_alg_power_two(PyObject *self, PyObject *args)
{
    double x;

    if (!PyArg_ParseTuple(args, "d", &x))
        return NULL;

    return PyFloat_FromDouble(sp_alg_power(2.0, x));
}

static PyObject *sp_alg_power_three(PyObject *self, PyObject *args)
{
    double x;

    if (!PyArg_ParseTuple(args, "d", &x))
        return NULL;

    return PyFloat_FromDouble(sp_alg_power(3.0, x));
}

static PyObject *sp_alg_power_of_x(PyObject *self, PyObject *args)
{
    double x, n;

    if (!PyArg_ParseTuple(args, "dd", &x, &n))
        return NULL;

    return PyFloat_FromDouble(sp_alg_power(x, n));
}

static PyObject *sp_alg_power_sum(PyObject *self, PyObject *args)
{
    double a, b, n;

    if (!PyArg_ParseTuple(args, "ddd", &a, &b, &n))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_power(a, n) + sp_alg_power(b, n)
    );
}

static PyObject *sp_alg_power_difference(PyObject *self, PyObject *args)
{
    double a, b, n;

    if (!PyArg_ParseTuple(args, "ddd", &a, &b, &n))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_power(a, n) - sp_alg_power(b, n)
    );
}

static PyObject *sp_alg_product_power(PyObject *self, PyObject *args)
{
    double a, b, n;

    if (!PyArg_ParseTuple(args, "ddd", &a, &b, &n))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_power(a * b, n)
    );
}

static PyObject *sp_alg_quotient_power(PyObject *self, PyObject *args)
{
    double a, b, n;

    if (!PyArg_ParseTuple(args, "ddd", &a, &b, &n))
        return NULL;

    if (b == 0.0)
    {
        PyErr_SetString(PyExc_ZeroDivisionError, "denominator cannot be zero");
        return NULL;
    }

    return PyFloat_FromDouble(
        sp_alg_power(a / b, n)
    );
}

static PyObject *sp_alg_power_product(PyObject *self, PyObject *args)
{
    double a, m, n;

    if (!PyArg_ParseTuple(args, "ddd", &a, &m, &n))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_power(a, m * n)
    );
}

static PyObject *sp_alg_power_quotient(PyObject *self, PyObject *args)
{
    double a, m, n;

    if (!PyArg_ParseTuple(args, "ddd", &a, &m, &n))
        return NULL;

    if (n == 0.0)
    {
        PyErr_SetString(PyExc_ZeroDivisionError, "exponent divisor cannot be zero");
        return NULL;
    }

    return PyFloat_FromDouble(
        sp_alg_power(a, m / n)
    );
}

static PyObject *sp_alg_power_zero(PyObject *self, PyObject *args)
{
    double a;

    if (!PyArg_ParseTuple(args, "d", &a))
        return NULL;

    if (a == 0.0)
    {
        PyErr_SetString(PyExc_ValueError, "0^0 is undefined");
        return NULL;
    }

    return PyFloat_FromDouble(1.0);
}

static PyObject *sp_alg_power_one(PyObject *self, PyObject *args)
{
    double a;

    if (!PyArg_ParseTuple(args, "d", &a))
        return NULL;

    return PyFloat_FromDouble(a);
}

static PyObject *sp_alg_square_plus_constant(PyObject *self, PyObject *args)
{
    double x, c;

    if (!PyArg_ParseTuple(args, "dd", &x, &c))
        return NULL;

    return PyFloat_FromDouble(sp_alg_square(x) + c);
}

static PyObject *sp_alg_square_minus_constant(PyObject *self, PyObject *args)
{
    double x, c;

    if (!PyArg_ParseTuple(args, "dd", &x, &c))
        return NULL;

    return PyFloat_FromDouble(sp_alg_square(x) - c);
}

static PyObject *sp_alg_cube_plus_constant(PyObject *self, PyObject *args)
{
    double x, c;

    if (!PyArg_ParseTuple(args, "dd", &x, &c))
        return NULL;

    return PyFloat_FromDouble(sp_alg_cube(x) + c);
}

static PyObject *sp_alg_cube_minus_constant(PyObject *self, PyObject *args)
{
    double x, c;

    if (!PyArg_ParseTuple(args, "dd", &x, &c))
        return NULL;

    return PyFloat_FromDouble(sp_alg_cube(x) - c);
}

static PyObject *sp_alg_scaled_square(PyObject *self, PyObject *args)
{
    double a, x;

    if (!PyArg_ParseTuple(args, "dd", &a, &x))
        return NULL;

    return PyFloat_FromDouble(a * sp_alg_square(x));
}

static PyObject *sp_alg_scaled_cube(PyObject *self, PyObject *args)
{
    double a, x;

    if (!PyArg_ParseTuple(args, "dd", &a, &x))
        return NULL;

    return PyFloat_FromDouble(a * sp_alg_cube(x));
}

static PyObject *sp_alg_scaled_power(PyObject *self, PyObject *args)
{
    double a, x, n;

    if (!PyArg_ParseTuple(args, "ddd", &a, &x, &n))
        return NULL;

    return PyFloat_FromDouble(a * sp_alg_power(x, n));
}

static PyObject *sp_alg_shift(PyObject *self, PyObject *args)
{
    double x, amount;

    if (!PyArg_ParseTuple(args, "dd", &x, &amount))
        return NULL;

    return PyFloat_FromDouble(x + amount);
}

static PyObject *sp_alg_shift_negative(PyObject *self, PyObject *args)
{
    double x, amount;

    if (!PyArg_ParseTuple(args, "dd", &x, &amount))
        return NULL;

    return PyFloat_FromDouble(x - amount);
}

static PyObject *sp_alg_absolute_sum(PyObject *self, PyObject *args)
{
    double a, b;

    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_abs(a) + sp_alg_abs(b)
    );
}

static PyObject *sp_alg_absolute_product(PyObject *self, PyObject *args)
{
    double a, b;

    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_abs(a) * sp_alg_abs(b)
    );
}

static PyObject *sp_alg_absolute_quotient(PyObject *self, PyObject *args)
{
    double a, b;

    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    if (b == 0.0)
    {
        PyErr_SetString(PyExc_ZeroDivisionError, "division by zero");
        return NULL;
    }

    return PyFloat_FromDouble(sp_alg_abs(a) / sp_alg_abs(b));
}

static PyObject *sp_alg_signed_value(PyObject *self, PyObject *args)
{
    double x, magnitude;

    if (!PyArg_ParseTuple(args, "dd", &x, &magnitude))
        return NULL;

    return PyFloat_FromDouble(sp_alg_sign(x) * sp_alg_abs(magnitude));
}

static PyObject *sp_alg_copy_sign(PyObject *self, PyObject *args)
{
    double x, sign_value;

    if (!PyArg_ParseTuple(args, "dd", &x, &sign_value))
        return NULL;

    return PyFloat_FromDouble(sp_alg_sign(sign_value) * sp_alg_abs(x));
}

static PyObject *sp_alg_positive_part(PyObject *self, PyObject *args)
{
    double x;

    if (!PyArg_ParseTuple(args, "d", &x))
        return NULL;

    return PyFloat_FromDouble((sp_alg_abs(x) + x) / 2.0);
}

static PyObject *sp_alg_negative_part(PyObject *self, PyObject *args)
{
    double x;

    if (!PyArg_ParseTuple(args, "d", &x))
        return NULL;

    return PyFloat_FromDouble((sp_alg_abs(x) - x) / 2.0);
}

static PyObject *sp_alg_signum_times_square(PyObject *self, PyObject *args)
{
    double x;

    if (!PyArg_ParseTuple(args, "d", &x))
        return NULL;

    return PyFloat_FromDouble(sp_alg_sign(x) * sp_alg_square(x));
}

static PyObject *sp_alg_signum_times_cube(PyObject *self, PyObject *args)
{
    double x;

    if (!PyArg_ParseTuple(args, "d", &x))
        return NULL;

    return PyFloat_FromDouble(sp_alg_sign(x) * sp_alg_cube(x));
}

static PyObject *sp_alg_signed_square(PyObject *self, PyObject *args)
{
    double x;

    if (!PyArg_ParseTuple(args, "d", &x))
        return NULL;

    return PyFloat_FromDouble(sp_alg_sign(x) * sp_alg_square(sp_alg_abs(x)));
}

static PyObject *sp_alg_signed_cube(PyObject *self, PyObject *args)
{
    double x;

    if (!PyArg_ParseTuple(args, "d", &x))
        return NULL;

    return PyFloat_FromDouble(sp_alg_sign(x) * sp_alg_cube(sp_alg_abs(x)));
}

static PyObject *sp_alg_floor_sum(PyObject *self, PyObject *args)
{
    double a, b;

    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    return PyFloat_FromDouble(sp_alg_floor(a) + sp_alg_floor(b));
}

static PyObject *sp_alg_floor_difference(PyObject *self, PyObject *args)
{
    double a, b;

    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    return PyFloat_FromDouble(sp_alg_floor(a) - sp_alg_floor(b));
}

static PyObject *sp_alg_ceil_sum(PyObject *self, PyObject *args)
{
    double a, b;

    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    return PyFloat_FromDouble(sp_alg_ceil(a) + sp_alg_ceil(b));
}

static PyObject *sp_alg_ceil_difference(PyObject *self, PyObject *args)
{
    double a, b;

    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    return PyFloat_FromDouble(sp_alg_ceil(a) - sp_alg_ceil(b));
}

static PyObject *sp_alg_floor_product(PyObject *self, PyObject *args)
{
    double a, b;

    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    return PyFloat_FromDouble(sp_alg_floor(a * b));
}

static PyObject *sp_alg_ceil_product(PyObject *self, PyObject *args)
{
    double a, b;

    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    return PyFloat_FromDouble(sp_alg_ceil(a * b));
}

static PyObject *sp_alg_floor_quotient(PyObject *self, PyObject *args)
{
    double a, b;

    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    if (b == 0.0)
    {
        PyErr_SetString(PyExc_ZeroDivisionError, "division by zero");
        return NULL;
    }

    return PyFloat_FromDouble(sp_alg_floor(a / b));
}

static PyObject *sp_alg_ceil_quotient(PyObject *self, PyObject *args)
{
    double a, b;

    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    if (b == 0.0)
    {
        PyErr_SetString(PyExc_ZeroDivisionError, "division by zero");
        return NULL;
    }

    return PyFloat_FromDouble(sp_alg_ceil(a / b));
}

static PyObject *sp_alg_floor_power(PyObject *self, PyObject *args)
{
    double a, b;

    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    return PyFloat_FromDouble(sp_alg_floor(sp_alg_power(a, b)));
}

static PyObject *sp_alg_ceil_power(PyObject *self, PyObject *args)
{
    double a, b;

    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    return PyFloat_FromDouble(sp_alg_ceil(sp_alg_power(a, b)));
}

static PyObject *sp_alg_floor_square(PyObject *self, PyObject *args)
{
    double x;

    if (!PyArg_ParseTuple(args, "d", &x))
        return NULL;

    return PyFloat_FromDouble(sp_alg_floor(sp_alg_square(x)));
}

static PyObject *sp_alg_ceil_square(PyObject *self, PyObject *args)
{
    double x;

    if (!PyArg_ParseTuple(args, "d", &x))
        return NULL;

    return PyFloat_FromDouble(sp_alg_ceil(sp_alg_square(x)));
}

static PyObject *sp_alg_floor_cube(PyObject *self, PyObject *args)
{
    double x;

    if (!PyArg_ParseTuple(args, "d", &x))
        return NULL;

    return PyFloat_FromDouble(sp_alg_floor(sp_alg_cube(x)));
}

static PyObject *sp_alg_ceil_cube(PyObject *self, PyObject *args)
{
    double x;

    if (!PyArg_ParseTuple(args, "d", &x))
        return NULL;

    return PyFloat_FromDouble(sp_alg_ceil(sp_alg_cube(x)));
}

static PyObject *sp_alg_mod_square(PyObject *self, PyObject *args)
{
    double x, m;

    if (!PyArg_ParseTuple(args, "dd", &x, &m))
        return NULL;

    if (m == 0.0)
    {
        PyErr_SetString(PyExc_ZeroDivisionError, "modulus cannot be zero");
        return NULL;
    }

    return PyFloat_FromDouble(sp_alg_mod(sp_alg_square(x), m));
}

static PyObject *sp_alg_mod_cube(PyObject *self, PyObject *args)
{
    double x, m;

    if (!PyArg_ParseTuple(args, "dd", &x, &m))
        return NULL;

    if (m == 0.0)
    {
        PyErr_SetString(PyExc_ZeroDivisionError, "modulus cannot be zero");
        return NULL;
    }

    return PyFloat_FromDouble(sp_alg_mod(sp_alg_cube(x), m));
}

static PyObject *sp_alg_mod_power(PyObject *self, PyObject *args)
{
    double x, n, m;

    if (!PyArg_ParseTuple(args, "ddd", &x, &n, &m))
        return NULL;

    if (m == 0.0)
    {
        PyErr_SetString(PyExc_ZeroDivisionError, "modulus cannot be zero");
        return NULL;
    }

    return PyFloat_FromDouble(
        sp_alg_mod(sp_alg_power(x, n), m)
    );
}

static PyObject *sp_alg_power_square(PyObject *self, PyObject *args)
{
    double x, n;

    if (!PyArg_ParseTuple(args, "dd", &x, &n))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_power(sp_alg_square(x), n)
    );
}

static PyObject *sp_alg_power_cube(PyObject *self, PyObject *args)
{
    double x, n;

    if (!PyArg_ParseTuple(args, "dd", &x, &n))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_power(sp_alg_cube(x), n)
    );
}

static PyObject *sp_alg_square_power(PyObject *self, PyObject *args)
{
    double x, n;

    if (!PyArg_ParseTuple(args, "dd", &x, &n))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_square(sp_alg_power(x, n))
    );
}

static PyObject *sp_alg_cube_power(PyObject *self, PyObject *args)
{
    double x, n;

    if (!PyArg_ParseTuple(args, "dd", &x, &n))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_cube(sp_alg_power(x, n))
    );
}

static PyObject *sp_alg_power_sum_two(PyObject *self, PyObject *args)
{
    double x, a, b;

    if (!PyArg_ParseTuple(args, "ddd", &x, &a, &b))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_power(x, a) + sp_alg_power(x, b)
    );
}

static PyObject *sp_alg_power_difference_two(PyObject *self, PyObject *args)
{
    double x, a, b;

    if (!PyArg_ParseTuple(args, "ddd", &x, &a, &b))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_power(x, a) - sp_alg_power(x, b)
    );
}

static PyObject *sp_alg_power_product_two(PyObject *self, PyObject *args)
{
    double x, a, b;

    if (!PyArg_ParseTuple(args, "ddd", &x, &a, &b))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_power(x, a * b)
    );
}

static PyObject *sp_alg_power_ratio_two(PyObject *self, PyObject *args)
{
    double x, a, b;

    if (!PyArg_ParseTuple(args, "ddd", &x, &a, &b))
        return NULL;

    if (b == 0.0)
    {
        PyErr_SetString(PyExc_ZeroDivisionError, "exponent divisor cannot be zero");
        return NULL;
    }

    return PyFloat_FromDouble(
        sp_alg_power(x, a / b)
    );
}

static PyObject *sp_alg_square_scaled_sum(PyObject *self, PyObject *args)
{
    double a, x, b;

    if (!PyArg_ParseTuple(args, "ddd", &a, &x, &b))
        return NULL;

    return PyFloat_FromDouble(
        a * sp_alg_square(x) + b
    );
}

static PyObject *sp_alg_square_scaled_difference(PyObject *self, PyObject *args)
{
    double a, x, b;

    if (!PyArg_ParseTuple(args, "ddd", &a, &x, &b))
        return NULL;

    return PyFloat_FromDouble(
        a * sp_alg_square(x) - b
    );
}

static PyObject *sp_alg_cube_scaled_sum(PyObject *self, PyObject *args)
{
    double a, x, b;

    if (!PyArg_ParseTuple(args, "ddd", &a, &x, &b))
        return NULL;

    return PyFloat_FromDouble(
        a * sp_alg_cube(x) + b
    );
}

static PyObject *sp_alg_cube_scaled_difference(PyObject *self, PyObject *args)
{
    double a, x, b;

    if (!PyArg_ParseTuple(args, "ddd", &a, &x, &b))
        return NULL;

    return PyFloat_FromDouble(
        a * sp_alg_cube(x) - b
    );
}

static PyObject *sp_alg_power_scaled_sum(PyObject *self, PyObject *args)
{
    double a, x, n, b;

    if (!PyArg_ParseTuple(args, "dddd", &a, &x, &n, &b))
        return NULL;

    return PyFloat_FromDouble(
        a * sp_alg_power(x, n) + b
    );
}

static PyObject *sp_alg_power_scaled_difference(PyObject *self, PyObject *args)
{
    double a, x, n, b;

    if (!PyArg_ParseTuple(args, "dddd", &a, &x, &n, &b))
        return NULL;

    return PyFloat_FromDouble(
        a * sp_alg_power(x, n) - b
    );
}

static PyObject *sp_alg_linear_transform(PyObject *self, PyObject *args)
{
    double a, x, b;

    if (!PyArg_ParseTuple(args, "ddd", &a, &x, &b))
        return NULL;

    return PyFloat_FromDouble(a * x + b);
}

static PyObject *sp_alg_affine_difference(PyObject *self, PyObject *args)
{
    double a, x, b, y;

    if (!PyArg_ParseTuple(args, "dddd", &a, &x, &b, &y))
        return NULL;

    return PyFloat_FromDouble(
        (a * x + b) - (a * y + b)
    );
}

static PyObject *sp_alg_affine_sum(PyObject *self, PyObject *args)
{
    double a, x, b, y;

    if (!PyArg_ParseTuple(args, "dddd", &a, &x, &b, &y))
        return NULL;

    return PyFloat_FromDouble(
        (a * x + b) + (a * y + b)
    );
}

static PyObject *sp_alg_affine_product(PyObject *self, PyObject *args)
{
    double a, x, b, y;

    if (!PyArg_ParseTuple(args, "dddd", &a, &x, &b, &y))
        return NULL;

    return PyFloat_FromDouble(
        (a * x + b) * (a * y + b)
    );
}

static PyObject *sp_alg_affine_square(PyObject *self, PyObject *args)
{
    double a, x, b;

    if (!PyArg_ParseTuple(args, "ddd", &a, &x, &b))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_square(a * x + b)
    );
}

static PyObject *sp_alg_affine_cube(PyObject *self, PyObject *args)
{
    double a, x, b;

    if (!PyArg_ParseTuple(args, "ddd", &a, &x, &b))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_cube(a * x + b)
    );
}

static PyObject *sp_alg_affine_power(PyObject *self, PyObject *args)
{
    double a, x, b, n;

    if (!PyArg_ParseTuple(args, "dddd", &a, &x, &b, &n))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_power(a * x + b, n)
    );
}

static PyObject *sp_alg_affine_abs(PyObject *self, PyObject *args)
{
    double a, x, b;

    if (!PyArg_ParseTuple(args, "ddd", &a, &x, &b))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_abs(a * x + b)
    );
}

static PyObject *sp_alg_affine_sign(PyObject *self, PyObject *args)
{
    double a, x, b;

    if (!PyArg_ParseTuple(args, "ddd", &a, &x, &b))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_sign(a * x + b)
    );
}

static PyObject *sp_alg_affine_floor(PyObject *self, PyObject *args)
{
    double a, x, b;

    if (!PyArg_ParseTuple(args, "ddd", &a, &x, &b))
        return NULL;

    return PyFloat_FromDouble(sp_alg_floor(a * x + b));
}

static PyObject *sp_alg_affine_ceil(PyObject *self, PyObject *args)
{
    double a, x, b;

    if (!PyArg_ParseTuple(args, "ddd", &a, &x, &b))
        return NULL;

    return PyFloat_FromDouble(sp_alg_ceil(a * x + b));
}

static PyObject *sp_alg_affine_mod(PyObject *self, PyObject *args)
{
    double a, x, b, m;

    if (!PyArg_ParseTuple(args, "dddd", &a, &x, &b, &m))
        return NULL;

    if (m == 0.0)
    {
        PyErr_SetString(PyExc_ZeroDivisionError, "modulus cannot be zero");
        return NULL;
    }

    return PyFloat_FromDouble(sp_alg_mod(a * x + b, m));
}

static PyObject *sp_alg_affine_double(PyObject *self, PyObject *args)
{
    double a, x, b;

    if (!PyArg_ParseTuple(args, "ddd", &a, &x, &b))
        return NULL;

    return PyFloat_FromDouble(2.0 * (a * x + b));
}

static PyObject *sp_alg_affine_half(PyObject *self, PyObject *args)
{
    double a, x, b;

    if (!PyArg_ParseTuple(args, "ddd", &a, &x, &b))
        return NULL;

    return PyFloat_FromDouble((a * x + b) / 2.0);
}

static PyObject *sp_alg_affine_increment(PyObject *self, PyObject *args)
{
    double a, x, b;

    if (!PyArg_ParseTuple(args, "ddd", &a, &x, &b))
        return NULL;

    return PyFloat_FromDouble(a * x + b + 1.0);
}

static PyObject *sp_alg_affine_decrement(PyObject *self, PyObject *args)
{
    double a, x, b;

    if (!PyArg_ParseTuple(args, "ddd", &a, &x, &b))
        return NULL;

    return PyFloat_FromDouble(a * x + b - 1.0);
}

static PyObject *sp_alg_two_variable_linear(PyObject *self, PyObject *args)
{
    double a, x, b, y, c;

    if (!PyArg_ParseTuple(args, "ddddd", &a, &x, &b, &y, &c))
        return NULL;

    return PyFloat_FromDouble(a * x + b * y + c);
}

static PyObject *sp_alg_two_variable_product(PyObject *self, PyObject *args)
{
    double a, x, b, y;

    if (!PyArg_ParseTuple(args, "dddd", &a, &x, &b, &y))
        return NULL;

    return PyFloat_FromDouble((a * x) * (b * y));
}

static PyObject *sp_alg_two_variable_square_sum(PyObject *self, PyObject *args)
{
    double x, y;

    if (!PyArg_ParseTuple(args, "dd", &x, &y))
        return NULL;

    return PyFloat_FromDouble(sp_alg_square(x + y));
}

static PyObject *sp_alg_two_variable_square_difference(PyObject *self, PyObject *args)
{
    double x, y;

    if (!PyArg_ParseTuple(args, "dd", &x, &y))
        return NULL;

    return PyFloat_FromDouble(sp_alg_square(x - y));
}

static PyObject *sp_alg_two_variable_cube_sum(PyObject *self, PyObject *args)
{
    double x, y;

    if (!PyArg_ParseTuple(args, "dd", &x, &y))
        return NULL;

    return PyFloat_FromDouble(sp_alg_cube(x + y));
}

static PyObject *sp_alg_two_variable_cube_difference(PyObject *self, PyObject *args)
{
    double x, y;

    if (!PyArg_ParseTuple(args, "dd", &x, &y))
        return NULL;

    return PyFloat_FromDouble(sp_alg_cube(x - y));
}

static PyObject *sp_alg_two_variable_power_sum(PyObject *self, PyObject *args)
{
    double x, y, n;

    if (!PyArg_ParseTuple(args, "ddd", &x, &y, &n))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_power(x, n) + sp_alg_power(y, n)
    );
}

static PyObject *sp_alg_two_variable_power_difference(PyObject *self, PyObject *args)
{
    double x, y, n;

    if (!PyArg_ParseTuple(args, "ddd", &x, &y, &n))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_power(x, n) - sp_alg_power(y, n)
    );
}

static PyObject *sp_alg_two_variable_product_sum(PyObject *self, PyObject *args)
{
    double x, y, a, b;

    if (!PyArg_ParseTuple(args, "dddd", &x, &y, &a, &b))
        return NULL;

    return PyFloat_FromDouble(x * y + a * b);
}

static PyObject *sp_alg_two_variable_product_difference(PyObject *self, PyObject *args)
{
    double x, y, a, b;

    if (!PyArg_ParseTuple(args, "dddd", &x, &y, &a, &b))
        return NULL;

    return PyFloat_FromDouble(x * y - a * b);
}

static PyObject *sp_alg_three_variable_linear(PyObject *self, PyObject *args)
{
    double a, x, b, y, c, z;

    if (!PyArg_ParseTuple(args, "dddddd", &a, &x, &b, &y, &c, &z))
        return NULL;

    return PyFloat_FromDouble(a * x + b * y + c * z);
}

static PyObject *sp_alg_three_variable_sum(PyObject *self, PyObject *args)
{
    double x, y, z;

    if (!PyArg_ParseTuple(args, "ddd", &x, &y, &z))
        return NULL;

    return PyFloat_FromDouble(x + y + z);
}

static PyObject *sp_alg_three_variable_product(PyObject *self, PyObject *args)
{
    double x, y, z;

    if (!PyArg_ParseTuple(args, "ddd", &x, &y, &z))
        return NULL;

    return PyFloat_FromDouble(x * y * z);
}

static PyObject *sp_alg_three_variable_average(PyObject *self, PyObject *args)
{
    double x, y, z;

    if (!PyArg_ParseTuple(args, "ddd", &x, &y, &z))
        return NULL;

    return PyFloat_FromDouble((x + y + z) / 3.0);
}

static PyObject *sp_alg_three_variable_square_sum(PyObject *self, PyObject *args)
{
    double x, y, z;

    if (!PyArg_ParseTuple(args, "ddd", &x, &y, &z))
        return NULL;

    return PyFloat_FromDouble(sp_alg_square(x + y + z));
}

static PyObject *sp_alg_three_variable_cube_sum(PyObject *self, PyObject *args)
{
    double x, y, z;

    if (!PyArg_ParseTuple(args, "ddd", &x, &y, &z))
        return NULL;

    return PyFloat_FromDouble(sp_alg_cube(x + y + z));
}

static PyObject *sp_alg_three_variable_power_sum(PyObject *self, PyObject *args)
{
    double x, y, z, n;

    if (!PyArg_ParseTuple(args, "dddd", &x, &y, &z, &n))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_power(x, n) +
        sp_alg_power(y, n) +
        sp_alg_power(z, n)
    );
}

static PyObject *sp_alg_three_variable_abs_sum(PyObject *self, PyObject *args)
{
    double x, y, z;

    if (!PyArg_ParseTuple(args, "ddd", &x, &y, &z))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_abs(x) +
        sp_alg_abs(y) +
        sp_alg_abs(z)
    );
}

static PyObject *sp_alg_three_variable_sign_sum(PyObject *self, PyObject *args)
{
    double x, y, z;

    if (!PyArg_ParseTuple(args, "ddd", &x, &y, &z))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_sign(x) +
        sp_alg_sign(y) +
        sp_alg_sign(z)
    );
}

static PyObject *sp_alg_three_variable_floor_sum(PyObject *self, PyObject *args)
{
    double x, y, z;

    if (!PyArg_ParseTuple(args, "ddd", &x, &y, &z))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_floor(x) +
        sp_alg_floor(y) +
        sp_alg_floor(z)
    );
}

static PyObject *sp_alg_three_variable_ceil_sum(PyObject *self, PyObject *args)
{
    double x, y, z;

    if (!PyArg_ParseTuple(args, "ddd", &x, &y, &z))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_ceil(x) +
        sp_alg_ceil(y) +
        sp_alg_ceil(z)
    );
}

static PyObject *sp_alg_three_variable_mod_sum(PyObject *self, PyObject *args)
{
    double x, y, z, m;

    if (!PyArg_ParseTuple(args, "dddd", &x, &y, &z, &m))
        return NULL;

    if (m == 0.0)
    {
        PyErr_SetString(PyExc_ZeroDivisionError, "modulus cannot be zero");
        return NULL;
    }

    return PyFloat_FromDouble(
        sp_alg_mod(x, m) +
        sp_alg_mod(y, m) +
        sp_alg_mod(z, m)
    );
}

static PyObject *sp_alg_three_variable_mod_product(PyObject *self, PyObject *args)
{
    double x, y, z, m;

    if (!PyArg_ParseTuple(args, "dddd", &x, &y, &z, &m))
        return NULL;

    if (m == 0.0)
    {
        PyErr_SetString(PyExc_ZeroDivisionError, "modulus cannot be zero");
        return NULL;
    }

    return PyFloat_FromDouble(
        sp_alg_mod(x * y * z, m)
    );
}

static PyObject *sp_alg_three_variable_square_product(PyObject *self, PyObject *args)
{
    double x, y, z;

    if (!PyArg_ParseTuple(args, "ddd", &x, &y, &z))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_square(x * y * z)
    );
}

static PyObject *sp_alg_three_variable_cube_product(PyObject *self, PyObject *args)
{
    double x, y, z;

    if (!PyArg_ParseTuple(args, "ddd", &x, &y, &z))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_cube(x * y * z)
    );
}

static PyObject *sp_alg_sum_of_squares_two(PyObject *self, PyObject *args)
{
    double x, y;

    if (!PyArg_ParseTuple(args, "dd", &x, &y))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_square(x) + sp_alg_square(y)
    );
}

static PyObject *sp_alg_difference_of_squares_two(PyObject *self, PyObject *args)
{
    double x, y;

    if (!PyArg_ParseTuple(args, "dd", &x, &y))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_square(x) - sp_alg_square(y)
    );
}

static PyObject *sp_alg_sum_of_cubes_two(PyObject *self, PyObject *args)
{
    double x, y;

    if (!PyArg_ParseTuple(args, "dd", &x, &y))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_cube(x) + sp_alg_cube(y)
    );
}

static PyObject *sp_alg_difference_of_cubes_two(PyObject *self, PyObject *args)
{
    double x, y;

    if (!PyArg_ParseTuple(args, "dd", &x, &y))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_cube(x) - sp_alg_cube(y)
    );
}

static PyObject *sp_alg_power_sum_three(PyObject *self, PyObject *args)
{
    double x, y, z, n;

    if (!PyArg_ParseTuple(args, "dddd", &x, &y, &z, &n))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_power(x, n) +
        sp_alg_power(y, n) +
        sp_alg_power(z, n)
    );
}

static PyObject *sp_alg_power_product_three(PyObject *self, PyObject *args)
{
    double x, y, z, n;

    if (!PyArg_ParseTuple(args, "dddd", &x, &y, &z, &n))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_power(x * y * z, n)
    );
}

static PyObject *sp_alg_power_sum_four(PyObject *self, PyObject *args)
{
    double a, b, c, d, n;

    if (!PyArg_ParseTuple(args, "ddddd", &a, &b, &c, &d, &n))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_power(a, n) +
        sp_alg_power(b, n) +
        sp_alg_power(c, n) +
        sp_alg_power(d, n)
    );
}

static PyObject *sp_alg_power_product_four(PyObject *self, PyObject *args)
{
    double a, b, c, d, n;

    if (!PyArg_ParseTuple(args, "ddddd", &a, &b, &c, &d, &n))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_power(a * b * c * d, n)
    );
}

static PyObject *sp_alg_abs_power(PyObject *self, PyObject *args)
{
    double x, n;

    if (!PyArg_ParseTuple(args, "dd", &x, &n))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_power(sp_alg_abs(x), n)
    );
}

static PyObject *sp_alg_signed_power(PyObject *self, PyObject *args)
{
    double x, n;

    if (!PyArg_ParseTuple(args, "dd", &x, &n))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_sign(x) * sp_alg_power(sp_alg_abs(x), n)
    );
}

static PyObject *sp_alg_floor_abs(PyObject *self, PyObject *args)
{
    double x;

    if (!PyArg_ParseTuple(args, "d", &x))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_floor(sp_alg_abs(x))
    );
}

static PyObject *sp_alg_ceil_abs(PyObject *self, PyObject *args)
{
    double x;

    if (!PyArg_ParseTuple(args, "d", &x))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_ceil(sp_alg_abs(x))
    );
}

static PyObject *sp_alg_mod_abs(PyObject *self, PyObject *args)
{
    double x, m;

    if (!PyArg_ParseTuple(args, "dd", &x, &m))
        return NULL;

    if (m == 0.0)
    {
        PyErr_SetString(PyExc_ZeroDivisionError, "modulus cannot be zero");
        return NULL;
    }

    return PyFloat_FromDouble(
        sp_alg_mod(sp_alg_abs(x), m)
    );
}

static PyObject *sp_alg_square_abs(PyObject *self, PyObject *args)
{
    double x;

    if (!PyArg_ParseTuple(args, "d", &x))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_square(sp_alg_abs(x))
    );
}

static PyObject *sp_alg_cube_abs(PyObject *self, PyObject *args)
{
    double x;

    if (!PyArg_ParseTuple(args, "d", &x))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_cube(sp_alg_abs(x))
    );
}

static PyObject *sp_alg_abs_square_sum(PyObject *self, PyObject *args)
{
    double a, b;

    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_square(sp_alg_abs(a) + sp_alg_abs(b))
    );
}

static PyObject *sp_alg_abs_square_difference(PyObject *self, PyObject *args)
{
    double a, b;

    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_square(sp_alg_abs(a) - sp_alg_abs(b))
    );
}

static PyObject *sp_alg_abs_cube_sum(PyObject *self, PyObject *args)
{
    double a, b;

    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_cube(sp_alg_abs(a) + sp_alg_abs(b))
    );
}

static PyObject *sp_alg_abs_cube_difference(PyObject *self, PyObject *args)
{
    double a, b;

    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_cube(sp_alg_abs(a) - sp_alg_abs(b))
    );
}

static PyObject *sp_alg_abs_power_sum(PyObject *self, PyObject *args)
{
    double a, b, n;

    if (!PyArg_ParseTuple(args, "ddd", &a, &b, &n))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_power(sp_alg_abs(a), n) +
        sp_alg_power(sp_alg_abs(b), n)
    );
}

static PyObject *sp_alg_abs_power_difference(PyObject *self, PyObject *args)
{
    double a, b, n;

    if (!PyArg_ParseTuple(args, "ddd", &a, &b, &n))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_power(sp_alg_abs(a), n) -
        sp_alg_power(sp_alg_abs(b), n)
    );
}

static PyObject *sp_alg_floor_square_sum(PyObject *self, PyObject *args)
{
    double a, b;

    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_floor(sp_alg_square(a) + sp_alg_square(b))
    );
}

static PyObject *sp_alg_ceil_square_sum(PyObject *self, PyObject *args)
{
    double a, b;

    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_ceil(sp_alg_square(a) + sp_alg_square(b))
    );
}

static PyObject *sp_alg_floor_cube_sum(PyObject *self, PyObject *args)
{
    double a, b;

    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_floor(sp_alg_cube(a) + sp_alg_cube(b))
    );
}

static PyObject *sp_alg_ceil_cube_sum(PyObject *self, PyObject *args)
{
    double a, b;

    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_ceil(sp_alg_cube(a) + sp_alg_cube(b))
    );
}

static PyObject *sp_alg_floor_power_sum(PyObject *self, PyObject *args)
{
    double a, b, n;

    if (!PyArg_ParseTuple(args, "ddd", &a, &b, &n))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_floor(
            sp_alg_power(a, n) +
            sp_alg_power(b, n)
        )
    );
}

static PyObject *sp_alg_ceil_power_sum(PyObject *self, PyObject *args)
{
    double a, b, n;

    if (!PyArg_ParseTuple(args, "ddd", &a, &b, &n))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_ceil(
            sp_alg_power(a, n) +
            sp_alg_power(b, n)
        )
    );
}

static PyObject *sp_alg_floor_power_difference(PyObject *self, PyObject *args)
{
    double a, b, n;

    if (!PyArg_ParseTuple(args, "ddd", &a, &b, &n))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_floor(
            sp_alg_power(a, n) -
            sp_alg_power(b, n)
        )
    );
}

static PyObject *sp_alg_ceil_power_difference(PyObject *self, PyObject *args)
{
    double a, b, n;

    if (!PyArg_ParseTuple(args, "ddd", &a, &b, &n))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_ceil(
            sp_alg_power(a, n) -
            sp_alg_power(b, n)
        )
    );
}

static PyObject *sp_alg_mod_square_sum(PyObject *self, PyObject *args)
{
    double a, b, m;

    if (!PyArg_ParseTuple(args, "ddd", &a, &b, &m))
        return NULL;

    if (m == 0.0)
    {
        PyErr_SetString(PyExc_ZeroDivisionError, "modulus cannot be zero");
        return NULL;
    }

    return PyFloat_FromDouble(
        sp_alg_mod(
            sp_alg_square(a) + sp_alg_square(b),
            m
        )
    );
}

static PyObject *sp_alg_mod_cube_sum(PyObject *self, PyObject *args)
{
    double a, b, m;

    if (!PyArg_ParseTuple(args, "ddd", &a, &b, &m))
        return NULL;

    if (m == 0.0)
    {
        PyErr_SetString(PyExc_ZeroDivisionError, "modulus cannot be zero");
        return NULL;
    }

    return PyFloat_FromDouble(
        sp_alg_mod(
            sp_alg_cube(a) + sp_alg_cube(b),
            m
        )
    );
}

static PyObject *sp_alg_mod_power_sum(PyObject *self, PyObject *args)
{
    double a, b, n, m;

    if (!PyArg_ParseTuple(args, "dddd", &a, &b, &n, &m))
        return NULL;

    if (m == 0.0)
    {
        PyErr_SetString(PyExc_ZeroDivisionError, "modulus cannot be zero");
        return NULL;
    }

    return PyFloat_FromDouble(
        sp_alg_mod(
            sp_alg_power(a, n) +
            sp_alg_power(b, n),
            m
        )
    );
}

static PyObject *sp_alg_mod_power_difference(PyObject *self, PyObject *args)
{
    double a, b, n, m;

    if (!PyArg_ParseTuple(args, "dddd", &a, &b, &n, &m))
        return NULL;

    if (m == 0.0)
    {
        PyErr_SetString(PyExc_ZeroDivisionError, "modulus cannot be zero");
        return NULL;
    }

    return PyFloat_FromDouble(
        sp_alg_mod(
            sp_alg_power(a, n) -
            sp_alg_power(b, n),
            m
        )
    );
}

static PyObject *sp_alg_square_ratio(PyObject *self, PyObject *args)
{
    double a, b;

    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    if (b == 0.0)
    {
        PyErr_SetString(PyExc_ZeroDivisionError, "denominator cannot be zero");
        return NULL;
    }

    return PyFloat_FromDouble(
        sp_alg_square(a / b)
    );
}

static PyObject *sp_alg_cube_ratio(PyObject *self, PyObject *args)
{
    double a, b;

    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    if (b == 0.0)
    {
        PyErr_SetString(PyExc_ZeroDivisionError, "denominator cannot be zero");
        return NULL;
    }

    return PyFloat_FromDouble(
        sp_alg_cube(a / b)
    );
}

static PyObject *sp_alg_power_ratio(PyObject *self, PyObject *args)
{
    double a, b, n;

    if (!PyArg_ParseTuple(args, "ddd", &a, &b, &n))
        return NULL;

    if (b == 0.0)
    {
        PyErr_SetString(PyExc_ZeroDivisionError, "denominator cannot be zero");
        return NULL;
    }

    return PyFloat_FromDouble(
        sp_alg_power(a / b, n)
    );
}

static PyObject *sp_alg_ratio_square(PyObject *self, PyObject *args)
{
    double a, b;

    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    if (b == 0.0)
    {
        PyErr_SetString(PyExc_ZeroDivisionError, "denominator cannot be zero");
        return NULL;
    }

    return PyFloat_FromDouble(
        sp_alg_square(a) / sp_alg_square(b)
    );
}

static PyObject *sp_alg_ratio_cube(PyObject *self, PyObject *args)
{
    double a, b;

    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    if (b == 0.0)
    {
        PyErr_SetString(PyExc_ZeroDivisionError, "denominator cannot be zero");
        return NULL;
    }

    return PyFloat_FromDouble(
        sp_alg_cube(a) / sp_alg_cube(b)
    );
}

static PyObject *sp_alg_power_ratio_of_powers(PyObject *self, PyObject *args)
{
    double a, b, m, n;

    if (!PyArg_ParseTuple(args, "dddd", &a, &b, &m, &n))
        return NULL;

    if (b == 0.0)
    {
        PyErr_SetString(PyExc_ZeroDivisionError, "base denominator cannot be zero");
        return NULL;
    }

    return PyFloat_FromDouble(
        sp_alg_power(a, m) /
        sp_alg_power(b, n)
    );
}

static PyObject *sp_alg_product_of_squares(PyObject *self, PyObject *args)
{
    double a, b;

    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_square(a) * sp_alg_square(b)
    );
}

static PyObject *sp_alg_product_of_cubes(PyObject *self, PyObject *args)
{
    double a, b;

    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_cube(a) * sp_alg_cube(b)
    );
}

static PyObject *sp_alg_square_of_product(PyObject *self, PyObject *args)
{
    double a, b;

    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_square(a * b)
    );
}

static PyObject *sp_alg_cube_of_product(PyObject *self, PyObject *args)
{
    double a, b;

    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_cube(a * b)
    );
}

static PyObject *sp_alg_square_of_quotient(PyObject *self, PyObject *args)
{
    double a, b;

    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    if (b == 0.0)
    {
        PyErr_SetString(PyExc_ZeroDivisionError, "denominator cannot be zero");
        return NULL;
    }

    return PyFloat_FromDouble(
        sp_alg_square(a / b)
    );
}

static PyObject *sp_alg_cube_of_quotient(PyObject *self, PyObject *args)
{
    double a, b;

    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    if (b == 0.0)
    {
        PyErr_SetString(PyExc_ZeroDivisionError, "denominator cannot be zero");
        return NULL;
    }

    return PyFloat_FromDouble(
        sp_alg_cube(a / b)
    );
}

static PyObject *sp_alg_power_of_product(PyObject *self, PyObject *args)
{
    double a, b, n;

    if (!PyArg_ParseTuple(args, "ddd", &a, &b, &n))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_power(a * b, n)
    );
}

static PyObject *sp_alg_power_of_quotient(PyObject *self, PyObject *args)
{
    double a, b, n;

    if (!PyArg_ParseTuple(args, "ddd", &a, &b, &n))
        return NULL;

    if (b == 0.0)
    {
        PyErr_SetString(PyExc_ZeroDivisionError, "denominator cannot be zero");
        return NULL;
    }

    return PyFloat_FromDouble(
        sp_alg_power(a / b, n)
    );
}

static PyObject *sp_alg_square_linear_sum(PyObject *self, PyObject *args)
{
    double a, x, b;

    if (!PyArg_ParseTuple(args, "ddd", &a, &x, &b))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_square(a * x + b)
    );
}

static PyObject *sp_alg_square_linear_difference(PyObject *self, PyObject *args)
{
    double a, x, b;

    if (!PyArg_ParseTuple(args, "ddd", &a, &x, &b))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_square(a * x - b)
    );
}

static PyObject *sp_alg_cube_linear_sum(PyObject *self, PyObject *args)
{
    double a, x, b;

    if (!PyArg_ParseTuple(args, "ddd", &a, &x, &b))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_cube(a * x + b)
    );
}

static PyObject *sp_alg_cube_linear_difference(PyObject *self, PyObject *args)
{
    double a, x, b;

    if (!PyArg_ParseTuple(args, "ddd", &a, &x, &b))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_cube(a * x - b)
    );
}

static PyObject *sp_alg_power_linear_sum(PyObject *self, PyObject *args)
{
    double a, x, b, n;

    if (!PyArg_ParseTuple(args, "dddd", &a, &x, &b, &n))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_power(a * x + b, n)
    );
}

static PyObject *sp_alg_power_linear_difference(PyObject *self, PyObject *args)
{
    double a, x, b, n;

    if (!PyArg_ParseTuple(args, "dddd", &a, &x, &b, &n))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_power(a * x - b, n)
    );
}

static PyObject *sp_alg_mod_linear_sum(PyObject *self, PyObject *args)
{
    double a, x, b, m;

    if (!PyArg_ParseTuple(args, "dddd", &a, &x, &b, &m))
        return NULL;

    if (m == 0.0)
    {
        PyErr_SetString(PyExc_ZeroDivisionError, "modulus cannot be zero");
        return NULL;
    }

    return PyFloat_FromDouble(
        sp_alg_mod(a * x + b, m)
    );
}

static PyObject *sp_alg_mod_linear_difference(PyObject *self, PyObject *args)
{
    double a, x, b, m;

    if (!PyArg_ParseTuple(args, "dddd", &a, &x, &b, &m))
        return NULL;

    if (m == 0.0)
    {
        PyErr_SetString(PyExc_ZeroDivisionError, "modulus cannot be zero");
        return NULL;
    }

    return PyFloat_FromDouble(
        sp_alg_mod(a * x - b, m)
    );
}

static PyObject *sp_alg_floor_linear_sum(PyObject *self, PyObject *args)
{
    double a, x, b;

    if (!PyArg_ParseTuple(args, "ddd", &a, &x, &b))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_floor(a * x + b)
    );
}

static PyObject *sp_alg_ceil_linear_sum(PyObject *self, PyObject *args)
{
    double a, x, b;

    if (!PyArg_ParseTuple(args, "ddd", &a, &x, &b))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_ceil(a * x + b)
    );
}

static PyObject *sp_alg_floor_linear_difference(PyObject *self, PyObject *args)
{
    double a, x, b;

    if (!PyArg_ParseTuple(args, "ddd", &a, &x, &b))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_floor(a * x - b)
    );
}

static PyObject *sp_alg_ceil_linear_difference(PyObject *self, PyObject *args)
{
    double a, x, b;

    if (!PyArg_ParseTuple(args, "ddd", &a, &x, &b))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_ceil(a * x - b)
    );
}

static PyObject *sp_alg_abs_linear_sum(PyObject *self, PyObject *args)
{
    double a, b;
    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    return PyFloat_FromDouble(sp_alg_abs(a + b));
}

static PyObject *sp_alg_abs_linear_difference(PyObject *self, PyObject *args)
{
    double a, b;
    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    return PyFloat_FromDouble(sp_alg_abs(a - b));
}

static PyObject *sp_alg_sign_linear_sum(PyObject *self, PyObject *args)
{
    double a, b;
    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    return PyFloat_FromDouble(sp_alg_sign(a + b));
}

static PyObject *sp_alg_sign_linear_difference(PyObject *self, PyObject *args)
{
    double a, b;
    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    return PyFloat_FromDouble(sp_alg_sign(a - b));
}

static PyObject *sp_alg_abs_product_sum(PyObject *self, PyObject *args)
{
    double a, b;
    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    return PyFloat_FromDouble(sp_alg_abs(a * b));
}

static PyObject *sp_alg_abs_product_difference(PyObject *self, PyObject *args)
{
    double a, b;
    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    return PyFloat_FromDouble(sp_alg_abs(a * b - a - b));
}

static PyObject *sp_alg_sign_product_sum(PyObject *self, PyObject *args)
{
    double a, b;
    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    return PyFloat_FromDouble(sp_alg_sign(a * b + a + b));
}

static PyObject *sp_alg_sign_product_difference(PyObject *self, PyObject *args)
{
    double a, b;
    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    return PyFloat_FromDouble(sp_alg_sign(a * b - a - b));
}

static PyObject *sp_alg_floor_product_sum(PyObject *self, PyObject *args)
{
    double a, b;
    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    return PyFloat_FromDouble(sp_alg_floor(a * b + a + b));
}

static PyObject *sp_alg_ceil_product_sum(PyObject *self, PyObject *args)
{
    double a, b;
    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    return PyFloat_FromDouble(sp_alg_ceil(a * b + a + b));
}

static PyObject *sp_alg_floor_product_difference(PyObject *self, PyObject *args)
{
    double a, b;
    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    return PyFloat_FromDouble(sp_alg_floor(a * b - a - b));
}

static PyObject *sp_alg_ceil_product_difference(PyObject *self, PyObject *args)
{
    double a, b;
    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    return PyFloat_FromDouble(sp_alg_ceil(a * b - a - b));
}

static PyObject *sp_alg_mod_product_sum(PyObject *self, PyObject *args)
{
    double a, b, m;
    if (!PyArg_ParseTuple(args, "ddd", &a, &b, &m))
        return NULL;

    if (m == 0.0)
    {
        PyErr_SetString(PyExc_ZeroDivisionError, "modulus cannot be zero");
        return NULL;
    }

    return PyFloat_FromDouble(sp_alg_mod(a * b + a + b, m));
}

static PyObject *sp_alg_mod_product_difference(PyObject *self, PyObject *args)
{
    double a, b, m;
    if (!PyArg_ParseTuple(args, "ddd", &a, &b, &m))
        return NULL;

    if (m == 0.0)
    {
        PyErr_SetString(PyExc_ZeroDivisionError, "modulus cannot be zero");
        return NULL;
    }

    return PyFloat_FromDouble(sp_alg_mod(a * b - a - b, m));
}

static PyObject *sp_alg_square_product_sum(PyObject *self, PyObject *args)
{
    double a, b;
    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    return PyFloat_FromDouble(sp_alg_square(a * b + a + b));
}

static PyObject *sp_alg_square_product_difference(PyObject *self, PyObject *args)
{
    double a, b;
    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    return PyFloat_FromDouble(sp_alg_square(a * b - a - b));
}

static PyObject *sp_alg_cube_product_sum(PyObject *self, PyObject *args)
{
    double a, b;
    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    return PyFloat_FromDouble(sp_alg_cube(a * b + a + b));
}

static PyObject *sp_alg_cube_product_difference(PyObject *self, PyObject *args)
{
    double a, b;
    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    return PyFloat_FromDouble(sp_alg_cube(a * b - a - b));
}

static PyObject *sp_alg_power_product_sum(PyObject *self, PyObject *args)
{
    double a, b, n;
    if (!PyArg_ParseTuple(args, "ddd", &a, &b, &n))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_power(a * b + a + b, n)
    );
}

static PyObject *sp_alg_power_product_difference(PyObject *self, PyObject *args)
{
    double a, b, n;
    if (!PyArg_ParseTuple(args, "ddd", &a, &b, &n))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_power(a * b - a - b, n)
    );
}

static PyObject *sp_alg_floor_abs_sum(PyObject *self, PyObject *args)
{
    double a, b;
    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_floor(sp_alg_abs(a) + sp_alg_abs(b))
    );
}

static PyObject *sp_alg_ceil_abs_sum(PyObject *self, PyObject *args)
{
    double a, b;
    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_ceil(sp_alg_abs(a) + sp_alg_abs(b))
    );
}

static PyObject *sp_alg_floor_abs_difference(PyObject *self, PyObject *args)
{
    double a, b;
    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_floor(sp_alg_abs(a) - sp_alg_abs(b))
    );
}

static PyObject *sp_alg_ceil_abs_difference(PyObject *self, PyObject *args)
{
    double a, b;
    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_ceil(sp_alg_abs(a) - sp_alg_abs(b))
    );
}

static PyObject *sp_alg_mod_abs_sum(PyObject *self, PyObject *args)
{
    double a, b, m;
    if (!PyArg_ParseTuple(args, "ddd", &a, &b, &m))
        return NULL;

    if (m == 0.0)
    {
        PyErr_SetString(PyExc_ZeroDivisionError, "modulus cannot be zero");
        return NULL;
    }

    return PyFloat_FromDouble(
        sp_alg_mod(sp_alg_abs(a) + sp_alg_abs(b), m)
    );
}

static PyObject *sp_alg_mod_abs_difference(PyObject *self, PyObject *args)
{
    double a, b, m;
    if (!PyArg_ParseTuple(args, "ddd", &a, &b, &m))
        return NULL;

    if (m == 0.0)
    {
        PyErr_SetString(PyExc_ZeroDivisionError, "modulus cannot be zero");
        return NULL;
    }

    return PyFloat_FromDouble(
        sp_alg_mod(sp_alg_abs(a) - sp_alg_abs(b), m)
    );
}

static PyObject *sp_alg_square_abs_sum_three(PyObject *self, PyObject *args)
{
    double a, b, c;
    if (!PyArg_ParseTuple(args, "ddd", &a, &b, &c))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_square(
            sp_alg_abs(a) +
            sp_alg_abs(b) +
            sp_alg_abs(c)
        )
    );
}

static PyObject *sp_alg_cube_abs_sum_three(PyObject *self, PyObject *args)
{
    double a, b, c;
    if (!PyArg_ParseTuple(args, "ddd", &a, &b, &c))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_cube(
            sp_alg_abs(a) +
            sp_alg_abs(b) +
            sp_alg_abs(c)
        )
    );
}

static PyObject *sp_alg_power_abs_sum_three(PyObject *self, PyObject *args)
{
    double a, b, c, n;
    if (!PyArg_ParseTuple(args, "dddd", &a, &b, &c, &n))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_power(
            sp_alg_abs(a) +
            sp_alg_abs(b) +
            sp_alg_abs(c),
            n
        )
    );
}

static PyObject *sp_alg_square_sum_three(PyObject *self, PyObject *args)
{
    double a, b, c;
    if (!PyArg_ParseTuple(args, "ddd", &a, &b, &c))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_square(a + b + c)
    );
}

static PyObject *sp_alg_cube_sum_three(PyObject *self, PyObject *args)
{
    double a, b, c;
    if (!PyArg_ParseTuple(args, "ddd", &a, &b, &c))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_cube(a + b + c)
    );
}

static PyObject *sp_alg_square_difference_three(PyObject *self, PyObject *args)
{
    double a, b, c;
    if (!PyArg_ParseTuple(args, "ddd", &a, &b, &c))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_square(a - b - c)
    );
}

static PyObject *sp_alg_cube_difference_three(PyObject *self, PyObject *args)
{
    double a, b, c;
    if (!PyArg_ParseTuple(args, "ddd", &a, &b, &c))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_cube(a - b - c)
    );
}

static PyObject *sp_alg_power_difference_three(PyObject *self, PyObject *args)
{
    double a, b, c, n;
    if (!PyArg_ParseTuple(args, "dddd", &a, &b, &c, &n))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_power(a - b - c, n)
    );
}

static PyObject *sp_alg_sum_then_square(PyObject *self, PyObject *args)
{
    double a, b;
    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_square(a + b)
    );
}

static PyObject *sp_alg_sum_then_cube(PyObject *self, PyObject *args)
{
    double a, b;
    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_cube(a + b)
    );
}

static PyObject *sp_alg_sum_then_power(PyObject *self, PyObject *args)
{
    double a, b, n;
    if (!PyArg_ParseTuple(args, "ddd", &a, &b, &n))
        return NULL;

    return PyFloat_FromDouble(
        sp_alg_power(a + b, n)
    );
}

#define SP_ALG_METHOD(name) \
    {#name, sp_alg_##name, METH_VARARGS, NULL}

static PyMethodDef algebra_methods[] = {
    SP_ALG_METHOD(add),
    SP_ALG_METHOD(subtract),
    SP_ALG_METHOD(multiply),
    SP_ALG_METHOD(divide),
    SP_ALG_METHOD(abs),
    SP_ALG_METHOD(sign),
    SP_ALG_METHOD(mod),
    SP_ALG_METHOD(floor),
    SP_ALG_METHOD(ceil),
    SP_ALG_METHOD(square),
    SP_ALG_METHOD(cube),
    SP_ALG_METHOD(power),
    SP_ALG_METHOD(negate),
    SP_ALG_METHOD(reciprocal),
    SP_ALG_METHOD(double),
    SP_ALG_METHOD(half),
    SP_ALG_METHOD(triple),
    SP_ALG_METHOD(quarter),
    SP_ALG_METHOD(increment),
    SP_ALG_METHOD(decrement),
    SP_ALG_METHOD(average_two),
    SP_ALG_METHOD(average_three),
    SP_ALG_METHOD(linear_value),
    SP_ALG_METHOD(linear_slope),
    SP_ALG_METHOD(linear_intercept),
    SP_ALG_METHOD(distance_on_number_line),
    SP_ALG_METHOD(midpoint),
    SP_ALG_METHOD(weighted_average),
    SP_ALG_METHOD(percentage),
    SP_ALG_METHOD(percent_of),
    SP_ALG_METHOD(ratio),
    SP_ALG_METHOD(proportion),
    SP_ALG_METHOD(sum_to_n),
    SP_ALG_METHOD(sum_squares_to_n),
    SP_ALG_METHOD(sum_cubes_to_n),
    SP_ALG_METHOD(square_difference),
    SP_ALG_METHOD(cube_difference),
    SP_ALG_METHOD(square_sum),
    SP_ALG_METHOD(cube_sum),

    SP_ALG_METHOD(difference_of_cubes),
    SP_ALG_METHOD(square_sum_value),
    SP_ALG_METHOD(square_difference_value),
    SP_ALG_METHOD(cube_sum_value),
    SP_ALG_METHOD(cube_difference_value),
    SP_ALG_METHOD(product_sum),
    SP_ALG_METHOD(product_difference),
    SP_ALG_METHOD(linear_combination),
    SP_ALG_METHOD(difference_linear_terms),
    SP_ALG_METHOD(sum_three),
    SP_ALG_METHOD(product_three),
    SP_ALG_METHOD(sum_four),
    SP_ALG_METHOD(product_four),
    SP_ALG_METHOD(mean_three),
    SP_ALG_METHOD(mean_four),
    SP_ALG_METHOD(range),
    SP_ALG_METHOD(absolute_difference),
    SP_ALG_METHOD(relative_difference),
    SP_ALG_METHOD(relative_error),
    SP_ALG_METHOD(percent_error),
    SP_ALG_METHOD(percent_change),
    SP_ALG_METHOD(increase_by_percent),
    SP_ALG_METHOD(decrease_by_percent),
    SP_ALG_METHOD(percent_increase),
    SP_ALG_METHOD(percent_decrease),
    SP_ALG_METHOD(scale),
    SP_ALG_METHOD(unscale),
    SP_ALG_METHOD(power_two),
    SP_ALG_METHOD(power_three),
    SP_ALG_METHOD(power_of_x),
    SP_ALG_METHOD(power_sum),
    SP_ALG_METHOD(power_difference),
    SP_ALG_METHOD(product_power),
    SP_ALG_METHOD(quotient_power),
    SP_ALG_METHOD(power_product),
    SP_ALG_METHOD(power_quotient),
    SP_ALG_METHOD(power_zero),
    SP_ALG_METHOD(power_one),
    SP_ALG_METHOD(square_plus_constant),
    SP_ALG_METHOD(square_minus_constant),
    SP_ALG_METHOD(cube_plus_constant),
    SP_ALG_METHOD(cube_minus_constant),
    SP_ALG_METHOD(scaled_square),
    SP_ALG_METHOD(scaled_cube),
    SP_ALG_METHOD(scaled_power),
    SP_ALG_METHOD(shift),
    SP_ALG_METHOD(shift_negative),
    SP_ALG_METHOD(absolute_sum),
    SP_ALG_METHOD(absolute_product),

    SP_ALG_METHOD(absolute_quotient),
    SP_ALG_METHOD(signed_value),
    SP_ALG_METHOD(copy_sign),
    SP_ALG_METHOD(positive_part),
    SP_ALG_METHOD(negative_part),
    SP_ALG_METHOD(signum_times_square),
    SP_ALG_METHOD(signum_times_cube),
    SP_ALG_METHOD(signed_square),
    SP_ALG_METHOD(signed_cube),
    SP_ALG_METHOD(floor_sum),
    SP_ALG_METHOD(floor_difference),
    SP_ALG_METHOD(ceil_sum),
    SP_ALG_METHOD(ceil_difference),
    SP_ALG_METHOD(floor_product),
    SP_ALG_METHOD(ceil_product),
    SP_ALG_METHOD(floor_quotient),
    SP_ALG_METHOD(ceil_quotient),
    SP_ALG_METHOD(floor_power),
    SP_ALG_METHOD(ceil_power),
    SP_ALG_METHOD(floor_square),
    SP_ALG_METHOD(ceil_square),
    SP_ALG_METHOD(floor_cube),
    SP_ALG_METHOD(ceil_cube),
    SP_ALG_METHOD(mod_square),
    SP_ALG_METHOD(mod_cube),
    SP_ALG_METHOD(mod_power),
    SP_ALG_METHOD(power_square),
    SP_ALG_METHOD(power_cube),
    SP_ALG_METHOD(square_power),
    SP_ALG_METHOD(cube_power),
    SP_ALG_METHOD(power_sum_two),
    SP_ALG_METHOD(power_difference_two),
    SP_ALG_METHOD(power_product_two),
    SP_ALG_METHOD(power_ratio_two),
    SP_ALG_METHOD(square_scaled_sum),
    SP_ALG_METHOD(square_scaled_difference),
    SP_ALG_METHOD(cube_scaled_sum),
    SP_ALG_METHOD(cube_scaled_difference),
    SP_ALG_METHOD(power_scaled_sum),
    SP_ALG_METHOD(power_scaled_difference),
    SP_ALG_METHOD(linear_transform),
    SP_ALG_METHOD(affine_difference),
    SP_ALG_METHOD(affine_sum),
    SP_ALG_METHOD(affine_product),
    SP_ALG_METHOD(affine_square),
    SP_ALG_METHOD(affine_cube),
    SP_ALG_METHOD(affine_power),
    SP_ALG_METHOD(affine_abs),
    SP_ALG_METHOD(affine_sign),

    SP_ALG_METHOD(affine_floor),
    SP_ALG_METHOD(affine_ceil),
    SP_ALG_METHOD(affine_mod),
    SP_ALG_METHOD(affine_double),
    SP_ALG_METHOD(affine_half),
    SP_ALG_METHOD(affine_increment),
    SP_ALG_METHOD(affine_decrement),
    SP_ALG_METHOD(two_variable_linear),
    SP_ALG_METHOD(two_variable_product),
    SP_ALG_METHOD(two_variable_square_sum),
    SP_ALG_METHOD(two_variable_square_difference),
    SP_ALG_METHOD(two_variable_cube_sum),
    SP_ALG_METHOD(two_variable_cube_difference),
    SP_ALG_METHOD(two_variable_power_sum),
    SP_ALG_METHOD(two_variable_power_difference),
    SP_ALG_METHOD(two_variable_product_sum),
    SP_ALG_METHOD(two_variable_product_difference),
    SP_ALG_METHOD(three_variable_linear),
    SP_ALG_METHOD(three_variable_sum),
    SP_ALG_METHOD(three_variable_product),
    SP_ALG_METHOD(three_variable_average),
    SP_ALG_METHOD(three_variable_square_sum),
    SP_ALG_METHOD(three_variable_cube_sum),
    SP_ALG_METHOD(three_variable_power_sum),
    SP_ALG_METHOD(three_variable_abs_sum),
    SP_ALG_METHOD(three_variable_sign_sum),
    SP_ALG_METHOD(three_variable_floor_sum),
    SP_ALG_METHOD(three_variable_ceil_sum),
    SP_ALG_METHOD(three_variable_mod_sum),
    SP_ALG_METHOD(three_variable_mod_product),
    SP_ALG_METHOD(three_variable_square_product),
    SP_ALG_METHOD(three_variable_cube_product),
    SP_ALG_METHOD(sum_of_squares_two),
    SP_ALG_METHOD(difference_of_squares_two),
    SP_ALG_METHOD(sum_of_cubes_two),
    SP_ALG_METHOD(difference_of_cubes_two),
    SP_ALG_METHOD(power_sum_three),
    SP_ALG_METHOD(power_product_three),
    SP_ALG_METHOD(power_sum_four),
    SP_ALG_METHOD(power_product_four),
    SP_ALG_METHOD(abs_power),
    SP_ALG_METHOD(signed_power),
    SP_ALG_METHOD(floor_abs),
    SP_ALG_METHOD(ceil_abs),
    SP_ALG_METHOD(mod_abs),
    SP_ALG_METHOD(square_abs),
    SP_ALG_METHOD(cube_abs),

    SP_ALG_METHOD(abs_square_sum),
    SP_ALG_METHOD(abs_square_difference),
    SP_ALG_METHOD(abs_cube_sum),
    SP_ALG_METHOD(abs_cube_difference),
    SP_ALG_METHOD(abs_power_sum),
    SP_ALG_METHOD(abs_power_difference),
    SP_ALG_METHOD(floor_square_sum),
    SP_ALG_METHOD(ceil_square_sum),
    SP_ALG_METHOD(floor_cube_sum),
    SP_ALG_METHOD(ceil_cube_sum),
    SP_ALG_METHOD(floor_power_sum),
    SP_ALG_METHOD(ceil_power_sum),
    SP_ALG_METHOD(floor_power_difference),
    SP_ALG_METHOD(ceil_power_difference),
    SP_ALG_METHOD(mod_square_sum),
    SP_ALG_METHOD(mod_cube_sum),
    SP_ALG_METHOD(mod_power_sum),
    SP_ALG_METHOD(mod_power_difference),
    SP_ALG_METHOD(square_ratio),
    SP_ALG_METHOD(cube_ratio),
    SP_ALG_METHOD(power_ratio),
    SP_ALG_METHOD(ratio_square),
    SP_ALG_METHOD(ratio_cube),
    SP_ALG_METHOD(power_ratio_of_powers),
    SP_ALG_METHOD(product_of_squares),
    SP_ALG_METHOD(product_of_cubes),
    SP_ALG_METHOD(square_of_product),
    SP_ALG_METHOD(cube_of_product),
    SP_ALG_METHOD(square_of_quotient),
    SP_ALG_METHOD(cube_of_quotient),
    SP_ALG_METHOD(power_of_product),
    SP_ALG_METHOD(power_of_quotient),
    SP_ALG_METHOD(square_linear_sum),
    SP_ALG_METHOD(square_linear_difference),
    SP_ALG_METHOD(cube_linear_sum),
    SP_ALG_METHOD(cube_linear_difference),
    SP_ALG_METHOD(power_linear_sum),
    SP_ALG_METHOD(power_linear_difference),
    SP_ALG_METHOD(mod_linear_sum),
    SP_ALG_METHOD(mod_linear_difference),
    SP_ALG_METHOD(floor_linear_sum),
    SP_ALG_METHOD(ceil_linear_sum),
    SP_ALG_METHOD(floor_linear_difference),
    SP_ALG_METHOD(ceil_linear_difference),

    SP_ALG_METHOD(abs_linear_sum),
    SP_ALG_METHOD(abs_linear_difference),
    SP_ALG_METHOD(sign_linear_sum),
    SP_ALG_METHOD(sign_linear_difference),
    SP_ALG_METHOD(abs_product_sum),
    SP_ALG_METHOD(abs_product_difference),
    SP_ALG_METHOD(sign_product_sum),
    SP_ALG_METHOD(sign_product_difference),
    SP_ALG_METHOD(floor_product_sum),
    SP_ALG_METHOD(ceil_product_sum),
    SP_ALG_METHOD(floor_product_difference),
    SP_ALG_METHOD(ceil_product_difference),
    SP_ALG_METHOD(mod_product_sum),
    SP_ALG_METHOD(mod_product_difference),
    SP_ALG_METHOD(square_product_sum),
    SP_ALG_METHOD(square_product_difference),
    SP_ALG_METHOD(cube_product_sum),
    SP_ALG_METHOD(cube_product_difference),
    SP_ALG_METHOD(power_product_sum),
    SP_ALG_METHOD(power_product_difference),
    SP_ALG_METHOD(floor_abs_sum),
    SP_ALG_METHOD(ceil_abs_sum),
    SP_ALG_METHOD(floor_abs_difference),
    SP_ALG_METHOD(ceil_abs_difference),
    SP_ALG_METHOD(mod_abs_sum),
    SP_ALG_METHOD(mod_abs_difference),
    SP_ALG_METHOD(square_abs_sum_three),
    SP_ALG_METHOD(cube_abs_sum_three),
    SP_ALG_METHOD(power_abs_sum_three),
    SP_ALG_METHOD(square_sum_three),
    SP_ALG_METHOD(cube_sum_three),
    SP_ALG_METHOD(square_difference_three),
    SP_ALG_METHOD(cube_difference_three),
    SP_ALG_METHOD(power_difference_three),
    SP_ALG_METHOD(sum_then_square),
    SP_ALG_METHOD(sum_then_cube),
    SP_ALG_METHOD(sum_then_power),

    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef algebra_module = {
    PyModuleDef_HEAD_INIT,
    "algebra",
    "Algebra functions for speedpy.",
    -1,
    algebra_methods
};

PyMODINIT_FUNC PyInit_algebra(void)
{
    return PyModule_Create(&algebra_module);
}

#undef SP_ALG_METHOD