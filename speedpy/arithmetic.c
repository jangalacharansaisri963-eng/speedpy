#include <Python.h>

static PyObject *sp_add(PyObject *self, PyObject *args)
{
    long long a, b;

    if (!PyArg_ParseTuple(args, "LL", &a, &b))
        return NULL;

    return PyLong_FromLongLong(a + b);
}

static PyObject *sp_subtract(PyObject *self, PyObject *args)
{
    long long a, b;

    if (!PyArg_ParseTuple(args, "LL", &a, &b))
        return NULL;

    return PyLong_FromLongLong(a - b);
}

static PyObject *sp_multiply(PyObject *self, PyObject *args)
{
    long long a, b;

    if (!PyArg_ParseTuple(args, "LL", &a, &b))
        return NULL;

    return PyLong_FromLongLong(a * b);
}

static PyObject *sp_power(PyObject *self, PyObject *args)
{
    long long base, exponent;
    long long result = 1;

    if (!PyArg_ParseTuple(args, "LL", &base, &exponent))
        return NULL;

    if (exponent < 0) {
        PyErr_SetString(
            PyExc_ValueError,
            "negative exponent is not supported for integer power"
        );
        return NULL;
    }

    while (exponent > 0) {
        if (exponent & 1)
            result *= base;

        exponent >>= 1;

        if (exponent)
            base *= base;
    }

    return PyLong_FromLongLong(result);
}

static PyObject *sp_square(PyObject *self, PyObject *args)
{
    long long x;

    if (!PyArg_ParseTuple(args, "L", &x))
        return NULL;

    return PyLong_FromLongLong(x * x);
}

static PyObject *sp_cube(PyObject *self, PyObject *args)
{
    long long x;

    if (!PyArg_ParseTuple(args, "L", &x))
        return NULL;

    return PyLong_FromLongLong(x * x * x);
}

static PyObject *sp_negate(PyObject *self, PyObject *args)
{
    long long x;

    if (!PyArg_ParseTuple(args, "L", &x))
        return NULL;

    return PyLong_FromLongLong(-x);
}

static PyObject *sp_absolute(PyObject *self, PyObject *args)
{
    long long x;

    if (!PyArg_ParseTuple(args, "L", &x))
        return NULL;

    if (x < 0)
        x = -x;

    return PyLong_FromLongLong(x);
}

static PyObject *sp_increment(PyObject *self, PyObject *args)
{
    long long x;

    if (!PyArg_ParseTuple(args, "L", &x))
        return NULL;

    return PyLong_FromLongLong(x + 1);
}

static PyObject *sp_decrement(PyObject *self, PyObject *args)
{
    long long x;

    if (!PyArg_ParseTuple(args, "L", &x))
        return NULL;

    return PyLong_FromLongLong(x - 1);
}

static PyObject *sp_double(PyObject *self, PyObject *args)
{
    long long x;

    if (!PyArg_ParseTuple(args, "L", &x))
        return NULL;

    return PyLong_FromLongLong(x * 2);
}

static PyObject *sp_triple(PyObject *self, PyObject *args)
{
    long long x;

    if (!PyArg_ParseTuple(args, "L", &x))
        return NULL;

    return PyLong_FromLongLong(x * 3);
}

static PyObject *sp_quadruple(PyObject *self, PyObject *args)
{
    long long x;

    if (!PyArg_ParseTuple(args, "L", &x))
        return NULL;

    return PyLong_FromLongLong(x * 4);
}

static PyObject *sp_half(PyObject *self, PyObject *args)
{
    long long x;

    if (!PyArg_ParseTuple(args, "L", &x))
        return NULL;

    return PyLong_FromLongLong(x / 2);
}

static PyObject *sp_third(PyObject *self, PyObject *args)
{
    long long x;

    if (!PyArg_ParseTuple(args, "L", &x))
        return NULL;

    return PyLong_FromLongLong(x / 3);
}

static PyObject *sp_remainder(PyObject *self, PyObject *args)
{
    long long a, b;

    if (!PyArg_ParseTuple(args, "LL", &a, &b))
        return NULL;

    if (b == 0) {
        PyErr_SetString(PyExc_ZeroDivisionError, "division by zero");
        return NULL;
    }

    return PyLong_FromLongLong(a % b);
}

static PyObject *sp_floor_divide(PyObject *self, PyObject *args)
{
    long long a, b;
    long long q;

    if (!PyArg_ParseTuple(args, "LL", &a, &b))
        return NULL;

    if (b == 0) {
        PyErr_SetString(PyExc_ZeroDivisionError, "division by zero");
        return NULL;
    }

    q = a / b;

    if ((a < 0) != (b < 0) && a % b != 0)
        q--;

    return PyLong_FromLongLong(q);
}

static PyObject *sp_is_even(PyObject *self, PyObject *args)
{
    long long x;

    if (!PyArg_ParseTuple(args, "L", &x))
        return NULL;

    if ((x & 1) == 0)
        Py_RETURN_TRUE;

    Py_RETURN_FALSE;
}

static PyObject *sp_is_odd(PyObject *self, PyObject *args)
{
    long long x;

    if (!PyArg_ParseTuple(args, "L", &x))
        return NULL;

    if ((x & 1) != 0)
        Py_RETURN_TRUE;

    Py_RETURN_FALSE;
}

static PyObject *sp_sign(PyObject *self, PyObject *args)
{
    long long x;

    if (!PyArg_ParseTuple(args, "L", &x))
        return NULL;

    if (x > 0)
        return PyLong_FromLongLong(1);

    if (x < 0)
        return PyLong_FromLongLong(-1);

    return PyLong_FromLongLong(0);
}

static PyObject *sp_min(PyObject *self, PyObject *args)
{
    long long a, b;

    if (!PyArg_ParseTuple(args, "LL", &a, &b))
        return NULL;

    return PyLong_FromLongLong(a < b ? a : b);
}

static PyObject *sp_max(PyObject *self, PyObject *args)
{
    long long a, b;

    if (!PyArg_ParseTuple(args, "LL", &a, &b))
        return NULL;

    return PyLong_FromLongLong(a > b ? a : b);
}

static PyObject *sp_clamp(PyObject *self, PyObject *args)
{
    long long value, minimum, maximum;

    if (!PyArg_ParseTuple(args, "LLL", &value, &minimum, &maximum))
        return NULL;

    if (minimum > maximum) {
        PyErr_SetString(
            PyExc_ValueError,
            "minimum cannot be greater than maximum"
        );
        return NULL;
    }

    if (value < minimum)
        value = minimum;
    else if (value > maximum)
        value = maximum;

    return PyLong_FromLongLong(value);
}

static PyObject *sp_difference(PyObject *self, PyObject *args)
{
    long long a, b;
    long long result;

    if (!PyArg_ParseTuple(args, "LL", &a, &b))
        return NULL;

    result = a - b;

    if (result < 0)
        result = -result;

    return PyLong_FromLongLong(result);
}

static PyObject *sp_sum_two(PyObject *self, PyObject *args)
{
    long long a, b;

    if (!PyArg_ParseTuple(args, "LL", &a, &b))
        return NULL;

    return PyLong_FromLongLong(a + b);
}

static PyObject *sp_product_two(PyObject *self, PyObject *args)
{
    long long a, b;

    if (!PyArg_ParseTuple(args, "LL", &a, &b))
        return NULL;

    return PyLong_FromLongLong(a * b);
}

static PyObject *sp_average_two(PyObject *self, PyObject *args)
{
    long long a, b;
    double result;

    if (!PyArg_ParseTuple(args, "LL", &a, &b))
        return NULL;

    result = ((double)a + (double)b) / 2.0;

    return PyFloat_FromDouble(result);
}

static PyObject *sp_midpoint(PyObject *self, PyObject *args)
{
    long long a, b;
    double result;

    if (!PyArg_ParseTuple(args, "LL", &a, &b))
        return NULL;

    result = (double)a + ((double)b - (double)a) / 2.0;

    return PyFloat_FromDouble(result);
}

static PyObject *sp_percent_of(PyObject *self, PyObject *args)
{
    long long percent, value;
    double result;

    if (!PyArg_ParseTuple(args, "LL", &percent, &value))
        return NULL;

    result = ((double)percent * (double)value) / 100.0;

    return PyFloat_FromDouble(result);
}

static PyObject *sp_percentage(PyObject *self, PyObject *args)
{
    long long part, whole;
    double result;

    if (!PyArg_ParseTuple(args, "LL", &part, &whole))
        return NULL;

    if (whole == 0) {
        PyErr_SetString(PyExc_ZeroDivisionError, "whole cannot be zero");
        return NULL;
    }

    result = ((double)part / (double)whole) * 100.0;

    return PyFloat_FromDouble(result);
}

static PyObject *sp_add_percent(PyObject *self, PyObject *args)
{
    long long value, percent;
    double result;

    if (!PyArg_ParseTuple(args, "LL", &value, &percent))
        return NULL;

    result = (double)value +
             ((double)value * (double)percent / 100.0);

    return PyFloat_FromDouble(result);
}

static PyObject *sp_subtract_percent(PyObject *self, PyObject *args)
{
    long long value, percent;
    double result;

    if (!PyArg_ParseTuple(args, "LL", &value, &percent))
        return NULL;

    result = (double)value -
             ((double)value * (double)percent / 100.0);

    return PyFloat_FromDouble(result);
}

static PyObject *sp_ratio(PyObject *self, PyObject *args)
{
    long long a, b;

    if (!PyArg_ParseTuple(args, "LL", &a, &b))
        return NULL;

    if (b == 0) {
        PyErr_SetString(PyExc_ZeroDivisionError, "ratio denominator is zero");
        return NULL;
    }

    return PyFloat_FromDouble((double)a / (double)b);
}

static PyObject *sp_reciprocal(PyObject *self, PyObject *args)
{
    long long x;

    if (!PyArg_ParseTuple(args, "L", &x))
        return NULL;

    if (x == 0) {
        PyErr_SetString(PyExc_ZeroDivisionError, "reciprocal of zero");
        return NULL;
    }

    return PyFloat_FromDouble(1.0 / (double)x);
}

static PyObject *sp_is_positive(PyObject *self, PyObject *args)
{
    long long x;

    if (!PyArg_ParseTuple(args, "L", &x))
        return NULL;

    if (x > 0)
        Py_RETURN_TRUE;

    Py_RETURN_FALSE;
}

static PyObject *sp_is_negative(PyObject *self, PyObject *args)
{
    long long x;

    if (!PyArg_ParseTuple(args, "L", &x))
        return NULL;

    if (x < 0)
        Py_RETURN_TRUE;

    Py_RETURN_FALSE;
}

static PyObject *sp_is_zero(PyObject *self, PyObject *args)
{
    long long x;

    if (!PyArg_ParseTuple(args, "L", &x))
        return NULL;

    if (x == 0)
        Py_RETURN_TRUE;

    Py_RETURN_FALSE;
}

static PyObject *sp_abs_difference(PyObject *self, PyObject *args)
{
    long long a, b;
    long long difference;

    if (!PyArg_ParseTuple(args, "LL", &a, &b))
        return NULL;

    difference = a - b;

    if (difference < 0)
        difference = -difference;

    return PyLong_FromLongLong(difference);
}

static PyObject *sp_increment_by(PyObject *self, PyObject *args)
{
    long long value, amount;

    if (!PyArg_ParseTuple(args, "LL", &value, &amount))
        return NULL;

    return PyLong_FromLongLong(value + amount);
}

static PyObject *sp_decrement_by(PyObject *self, PyObject *args)
{
    long long value, amount;

    if (!PyArg_ParseTuple(args, "LL", &value, &amount))
        return NULL;

    return PyLong_FromLongLong(value - amount);
}

static PyObject *sp_add_three(PyObject *self, PyObject *args)
{
    long long a, b, c;

    if (!PyArg_ParseTuple(args, "LLL", &a, &b, &c))
        return NULL;

    return PyLong_FromLongLong(a + b + c);
}

static PyObject *sp_sum_range(PyObject *self, PyObject *args)
{
    long long start, end;
    long long sum = 0;
    long long i;

    if (!PyArg_ParseTuple(args, "LL", &start, &end))
        return NULL;

    if (start <= end) {
        for (i = start; i <= end; i++)
            sum += i;
    } else {
        for (i = start; i >= end; i--)
            sum += i;
    }

    return PyLong_FromLongLong(sum);
}

static PyObject *sp_count_range(PyObject *self, PyObject *args)
{
    long long start, end;
    unsigned long long count;

    if (!PyArg_ParseTuple(args, "LL", &start, &end))
        return NULL;

    if (start <= end)
        count = (unsigned long long)end - (unsigned long long)start + 1;
    else
        count = (unsigned long long)start - (unsigned long long)end + 1;

    return PyLong_FromUnsignedLongLong(count);
}

static PyObject *sp_distance(PyObject *self, PyObject *args)
{
    long long a, b;
    long long distance;

    if (!PyArg_ParseTuple(args, "LL", &a, &b))
        return NULL;

    distance = a - b;

    if (distance < 0)
        distance = -distance;

    return PyLong_FromLongLong(distance);
}

static PyObject *sp_nearest_multiple(PyObject *self, PyObject *args)
{
    long long value, multiple;
    long long lower, upper;

    if (!PyArg_ParseTuple(args, "LL", &value, &multiple))
        return NULL;

    if (multiple == 0) {
        PyErr_SetString(
            PyExc_ZeroDivisionError,
            "multiple cannot be zero"
        );
        return NULL;
    }

    if (multiple < 0)
        multiple = -multiple;

    lower = (value / multiple) * multiple;

    if (value < 0 && value % multiple != 0)
        lower -= multiple;

    upper = lower + multiple;

    if (value - lower < upper - value)
        return PyLong_FromLongLong(lower);

    return PyLong_FromLongLong(upper);
}

static PyObject *sp_round_down_multiple(PyObject *self, PyObject *args)
{
    long long value, multiple;
    long long result;

    if (!PyArg_ParseTuple(args, "LL", &value, &multiple))
        return NULL;

    if (multiple == 0) {
        PyErr_SetString(
            PyExc_ZeroDivisionError,
            "multiple cannot be zero"
        );
        return NULL;
    }

    if (multiple < 0)
        multiple = -multiple;

    result = value / multiple;

    if (value < 0 && value % multiple != 0)
        result--;

    return PyLong_FromLongLong(result * multiple);
}

static PyObject *sp_round_up_multiple(PyObject *self, PyObject *args)
{
    long long value, multiple;
    long long result;

    if (!PyArg_ParseTuple(args, "LL", &value, &multiple))
        return NULL;

    if (multiple == 0) {
        PyErr_SetString(
            PyExc_ZeroDivisionError,
            "multiple cannot be zero"
        );
        return NULL;
    }

    if (multiple < 0)
        multiple = -multiple;

    result = value / multiple;

    if (value > 0 && value % multiple != 0)
        result++;

    return PyLong_FromLongLong(result * multiple);
}

static PyObject *sp_mod_positive(PyObject *self, PyObject *args)
{
    long long value, modulus;
    long long result;

    if (!PyArg_ParseTuple(args, "LL", &value, &modulus))
        return NULL;

    if (modulus <= 0) {
        PyErr_SetString(
            PyExc_ValueError,
            "modulus must be positive"
        );
        return NULL;
    }

    result = value % modulus;

    if (result < 0)
        result += modulus;

    return PyLong_FromLongLong(result);
}

static PyObject *sp_is_multiple(PyObject *self, PyObject *args)
{
    long long value, multiple;

    if (!PyArg_ParseTuple(args, "LL", &value, &multiple))
        return NULL;

    if (multiple == 0) {
        PyErr_SetString(
            PyExc_ValueError,
            "multiple cannot be zero"
        );
        return NULL;
    }

    if (value % multiple == 0)
        Py_RETURN_TRUE;

    Py_RETURN_FALSE;
}

static PyObject *sp_is_divisible_by(PyObject *self, PyObject *args)
{
    long long value, divisor;

    if (!PyArg_ParseTuple(args, "LL", &value, &divisor))
        return NULL;

    if (divisor == 0) {
        PyErr_SetString(
            PyExc_ZeroDivisionError,
            "divisor cannot be zero"
        );
        return NULL;
    }

    if (value % divisor == 0)
        Py_RETURN_TRUE;

    Py_RETURN_FALSE;
}

static PyObject *sp_divide_exact(PyObject *self, PyObject *args)
{
    long long value, divisor;

    if (!PyArg_ParseTuple(args, "LL", &value, &divisor))
        return NULL;

    if (divisor == 0) {
        PyErr_SetString(
            PyExc_ZeroDivisionError,
            "divisor cannot be zero"
        );
        return NULL;
    }

    if (value % divisor != 0) {
        PyErr_SetString(
            PyExc_ValueError,
            "value is not exactly divisible"
        );
        return NULL;
    }

    return PyLong_FromLongLong(value / divisor);
}

static PyObject *sp_greater(PyObject *self, PyObject *args)
{
    long long a, b;

    if (!PyArg_ParseTuple(args, "LL", &a, &b))
        return NULL;

    if (a > b)
        Py_RETURN_TRUE;

    Py_RETURN_FALSE;
}

static PyObject *sp_less(PyObject *self, PyObject *args)
{
    long long a, b;

    if (!PyArg_ParseTuple(args, "LL", &a, &b))
        return NULL;

    if (a < b)
        Py_RETURN_TRUE;

    Py_RETURN_FALSE;
}

static PyObject *sp_equal(PyObject *self, PyObject *args)
{
    long long a, b;

    if (!PyArg_ParseTuple(args, "LL", &a, &b))
        return NULL;

    if (a == b)
        Py_RETURN_TRUE;

    Py_RETURN_FALSE;
}

static PyObject *sp_not_equal(PyObject *self, PyObject *args)
{
    long long a, b;

    if (!PyArg_ParseTuple(args, "LL", &a, &b))
        return NULL;

    if (a != b)
        Py_RETURN_TRUE;

    Py_RETURN_FALSE;
}

static PyObject *sp_greater_equal(PyObject *self, PyObject *args)
{
    long long a, b;

    if (!PyArg_ParseTuple(args, "LL", &a, &b))
        return NULL;

    if (a >= b)
        Py_RETURN_TRUE;

    Py_RETURN_FALSE;
}

static PyObject *sp_less_equal(PyObject *self, PyObject *args)
{
    long long a, b;

    if (!PyArg_ParseTuple(args, "LL", &a, &b))
        return NULL;

    if (a <= b)
        Py_RETURN_TRUE;

    Py_RETURN_FALSE;
}

static PyObject *sp_between(PyObject *self, PyObject *args)
{
    long long value, minimum, maximum;

    if (!PyArg_ParseTuple(args, "LLL", &value, &minimum, &maximum))
        return NULL;

    if (minimum > maximum) {
        PyErr_SetString(
            PyExc_ValueError,
            "minimum cannot be greater than maximum"
        );
        return NULL;
    }

    if (value >= minimum && value <= maximum)
        Py_RETURN_TRUE;

    Py_RETURN_FALSE;
}

static PyObject *sp_is_integer_value(PyObject *self, PyObject *args)
{
    long long value;

    if (!PyArg_ParseTuple(args, "L", &value))
        return NULL;

    Py_RETURN_TRUE;
}

static PyObject *sp_power_of_two(PyObject *self, PyObject *args)
{
    long long exponent;
    long long result = 1;

    if (!PyArg_ParseTuple(args, "L", &exponent))
        return NULL;

    if (exponent < 0) {
        PyErr_SetString(
            PyExc_ValueError,
            "exponent must be non-negative"
        );
        return NULL;
    }

    while (exponent > 0) {
        result *= 2;
        exponent--;
    }

    return PyLong_FromLongLong(result);
}

static PyObject *sp_times_ten(PyObject *self, PyObject *args)
{
    long long value;

    if (!PyArg_ParseTuple(args, "L", &value))
        return NULL;

    return PyLong_FromLongLong(value * 10);
}

static PyObject *sp_times_hundred(PyObject *self, PyObject *args)
{
    long long value;

    if (!PyArg_ParseTuple(args, "L", &value))
        return NULL;

    return PyLong_FromLongLong(value * 100);
}

static PyObject *sp_times_thousand(PyObject *self, PyObject *args)
{
    long long value;

    if (!PyArg_ParseTuple(args, "L", &value))
        return NULL;

    return PyLong_FromLongLong(value * 1000);
}

static PyObject *sp_add_ten(PyObject *self, PyObject *args)
{
    long long value;

    if (!PyArg_ParseTuple(args, "L", &value))
        return NULL;

    return PyLong_FromLongLong(value + 10);
}

static PyObject *sp_subtract_ten(PyObject *self, PyObject *args)
{
    long long value;

    if (!PyArg_ParseTuple(args, "L", &value))
        return NULL;

    return PyLong_FromLongLong(value - 10);
}

static PyObject *sp_add_hundred(PyObject *self, PyObject *args)
{
    long long value;

    if (!PyArg_ParseTuple(args, "L", &value))
        return NULL;

    return PyLong_FromLongLong(value + 100);
}

static PyObject *sp_subtract_hundred(PyObject *self, PyObject *args)
{
    long long value;

    if (!PyArg_ParseTuple(args, "L", &value))
        return NULL;

    return PyLong_FromLongLong(value - 100);
}

static PyObject *sp_add_thousand(PyObject *self, PyObject *args)
{
    long long value;

    if (!PyArg_ParseTuple(args, "L", &value))
        return NULL;

    return PyLong_FromLongLong(value + 1000);
}

static PyObject *sp_subtract_thousand(PyObject *self, PyObject *args)
{
    long long value;

    if (!PyArg_ParseTuple(args, "L", &value))
        return NULL;

    return PyLong_FromLongLong(value - 1000);
}

static PyObject *sp_multiply_by_zero(PyObject *self, PyObject *args)
{
    long long value;

    if (!PyArg_ParseTuple(args, "L", &value))
        return NULL;

    return PyLong_FromLongLong(0);
}

static PyObject *sp_multiply_by_one(PyObject *self, PyObject *args)
{
    long long value;

    if (!PyArg_ParseTuple(args, "L", &value))
        return NULL;

    return PyLong_FromLongLong(value);
}

static PyObject *sp_multiply_by_five(PyObject *self, PyObject *args)
{
    long long value;

    if (!PyArg_ParseTuple(args, "L", &value))
        return NULL;

    return PyLong_FromLongLong(value * 5);
}

static PyObject *sp_multiply_by_ten(PyObject *self, PyObject *args)
{
    long long value;

    if (!PyArg_ParseTuple(args, "L", &value))
        return NULL;

    return PyLong_FromLongLong(value * 10);
}

static PyObject *sp_multiply_by_hundred(PyObject *self, PyObject *args)
{
    long long value;

    if (!PyArg_ParseTuple(args, "L", &value))
        return NULL;

    return PyLong_FromLongLong(value * 100);
}

static PyObject *sp_divide_by_one(PyObject *self, PyObject *args)
{
    long long value;

    if (!PyArg_ParseTuple(args, "L", &value))
        return NULL;

    return PyLong_FromLongLong(value);
}

static PyObject *sp_divide_by_two(PyObject *self, PyObject *args)
{
    long long value;

    if (!PyArg_ParseTuple(args, "L", &value))
        return NULL;

    return PyLong_FromLongLong(value / 2);
}

static PyObject *sp_divide_by_five(PyObject *self, PyObject *args)
{
    long long value;

    if (!PyArg_ParseTuple(args, "L", &value))
        return NULL;

    return PyLong_FromLongLong(value / 5);
}

static PyObject *sp_divide_by_ten(PyObject *self, PyObject *args)
{
    long long value;

    if (!PyArg_ParseTuple(args, "L", &value))
        return NULL;

    return PyLong_FromLongLong(value / 10);
}

static PyObject *sp_divide_by_hundred(PyObject *self, PyObject *args)
{
    long long value;

    if (!PyArg_ParseTuple(args, "L", &value))
        return NULL;

    return PyLong_FromLongLong(value / 100);
}

static PyObject *sp_remainder_positive(PyObject *self, PyObject *args)
{
    long long value, divisor;
    long long result;

    if (!PyArg_ParseTuple(args, "LL", &value, &divisor))
        return NULL;

    if (divisor <= 0) {
        PyErr_SetString(
            PyExc_ValueError,
            "divisor must be positive"
        );
        return NULL;
    }

    result = value % divisor;

    if (result < 0)
        result += divisor;

    return PyLong_FromLongLong(result);
}

static PyObject *sp_quotient(PyObject *self, PyObject *args)
{
    long long dividend, divisor;

    if (!PyArg_ParseTuple(args, "LL", &dividend, &divisor))
        return NULL;

    if (divisor == 0) {
        PyErr_SetString(
            PyExc_ZeroDivisionError,
            "division by zero"
        );
        return NULL;
    }

    return PyLong_FromLongLong(dividend / divisor);
}

static PyObject *sp_divmod_quotient(PyObject *self, PyObject *args)
{
    long long dividend, divisor;
    long long quotient;

    if (!PyArg_ParseTuple(args, "LL", &dividend, &divisor))
        return NULL;

    if (divisor == 0) {
        PyErr_SetString(
            PyExc_ZeroDivisionError,
            "division by zero"
        );
        return NULL;
    }

    quotient = dividend / divisor;

    return PyLong_FromLongLong(quotient);
}

static PyObject *sp_divmod_remainder(PyObject *self, PyObject *args)
{
    long long dividend, divisor;
    long long remainder;

    if (!PyArg_ParseTuple(args, "LL", &dividend, &divisor))
        return NULL;

    if (divisor == 0) {
        PyErr_SetString(
            PyExc_ZeroDivisionError,
            "division by zero"
        );
        return NULL;
    }

    remainder = dividend % divisor;

    return PyLong_FromLongLong(remainder);
}

static PyObject *sp_average_three(PyObject *self, PyObject *args)
{
    long long a, b, c;
    double result;

    if (!PyArg_ParseTuple(args, "LLL", &a, &b, &c))
        return NULL;

    result = ((double)a + (double)b + (double)c) / 3.0;

    return PyFloat_FromDouble(result);
}

static PyObject *sp_average_four(PyObject *self, PyObject *args)
{
    long long a, b, c, d;
    double result;

    if (!PyArg_ParseTuple(args, "LLLL", &a, &b, &c, &d))
        return NULL;

    result = (
        (double)a +
        (double)b +
        (double)c +
        (double)d
    ) / 4.0;

    return PyFloat_FromDouble(result);
}

static PyObject *sp_sum_three(PyObject *self, PyObject *args)
{
    long long a, b, c;

    if (!PyArg_ParseTuple(args, "LLL", &a, &b, &c))
        return NULL;

    return PyLong_FromLongLong(a + b + c);
}

static PyObject *sp_sum_four(PyObject *self, PyObject *args)
{
    long long a, b, c, d;

    if (!PyArg_ParseTuple(args, "LLLL", &a, &b, &c, &d))
        return NULL;

    return PyLong_FromLongLong(a + b + c + d);
}

static PyObject *sp_product_three(PyObject *self, PyObject *args)
{
    long long a, b, c;

    if (!PyArg_ParseTuple(args, "LLL", &a, &b, &c))
        return NULL;

    return PyLong_FromLongLong(a * b * c);
}

static PyObject *sp_product_four(PyObject *self, PyObject *args)
{
    long long a, b, c, d;

    if (!PyArg_ParseTuple(args, "LLLL", &a, &b, &c, &d))
        return NULL;

    return PyLong_FromLongLong(a * b * c * d);
}

static PyObject *sp_min_three(PyObject *self, PyObject *args)
{
    long long a, b, c;
    long long result;

    if (!PyArg_ParseTuple(args, "LLL", &a, &b, &c))
        return NULL;

    result = a;

    if (b < result)
        result = b;

    if (c < result)
        result = c;

    return PyLong_FromLongLong(result);
}

static PyObject *sp_max_three(PyObject *self, PyObject *args)
{
    long long a, b, c;
    long long result;

    if (!PyArg_ParseTuple(args, "LLL", &a, &b, &c))
        return NULL;

    result = a;

    if (b > result)
        result = b;

    if (c > result)
        result = c;

    return PyLong_FromLongLong(result);
}

static PyObject *sp_min_four(PyObject *self, PyObject *args)
{
    long long a, b, c, d;
    long long result;

    if (!PyArg_ParseTuple(args, "LLLL", &a, &b, &c, &d))
        return NULL;

    result = a;

    if (b < result)
        result = b;

    if (c < result)
        result = c;

    if (d < result)
        result = d;

    return PyLong_FromLongLong(result);
}

static PyObject *sp_max_four(PyObject *self, PyObject *args)
{
    long long a, b, c, d;
    long long result;

    if (!PyArg_ParseTuple(args, "LLLL", &a, &b, &c, &d))
        return NULL;

    result = a;

    if (b > result)
        result = b;

    if (c > result)
        result = c;

    if (d > result)
        result = d;

    return PyLong_FromLongLong(result);
}

static PyObject *sp_range_sum_formula(PyObject *self, PyObject *args)
{
    long long start, end;
    long long count;
    long long result;

    if (!PyArg_ParseTuple(args, "LL", &start, &end))
        return NULL;

    if (start > end) {
        PyErr_SetString(
            PyExc_ValueError,
            "start cannot be greater than end"
        );
        return NULL;
    }

    count = end - start + 1;

    result = (start + end) * count / 2;

    return PyLong_FromLongLong(result);
}

static PyObject *sp_triangular_number(PyObject *self, PyObject *args)
{
    long long n;

    if (!PyArg_ParseTuple(args, "L", &n))
        return NULL;

    if (n < 0) {
        PyErr_SetString(
            PyExc_ValueError,
            "n must be non-negative"
        );
        return NULL;
    }

    return PyLong_FromLongLong(n * (n + 1) / 2);
}

static PyObject *sp_double_factorial_simple(PyObject *self, PyObject *args)
{
    long long n;
    long long result = 1;

    if (!PyArg_ParseTuple(args, "L", &n))
        return NULL;

    if (n < 0) {
        PyErr_SetString(
            PyExc_ValueError,
            "n must be non-negative"
        );
        return NULL;
    }

    while (n > 1) {
        result *= n;
        n -= 2;
    }

    return PyLong_FromLongLong(result);
}

static PyObject *sp_square_plus_one(PyObject *self, PyObject *args)
{
    long long x;

    if (!PyArg_ParseTuple(args, "L", &x))
        return NULL;

    return PyLong_FromLongLong(x * x + 1);
}

static PyObject *sp_square_minus_one(PyObject *self, PyObject *args)
{
    long long x;

    if (!PyArg_ParseTuple(args, "L", &x))
        return NULL;

    return PyLong_FromLongLong(x * x - 1);
}

static PyObject *sp_cube_plus_one(PyObject *self, PyObject *args)
{
    long long x;

    if (!PyArg_ParseTuple(args, "L", &x))
        return NULL;

    return PyLong_FromLongLong(x * x * x + 1);
}

static PyObject *sp_cube_minus_one(PyObject *self, PyObject *args)
{
    long long x;

    if (!PyArg_ParseTuple(args, "L", &x))
        return NULL;

    return PyLong_FromLongLong(x * x * x - 1);
}

static PyObject *sp_double_plus_one(PyObject *self, PyObject *args)
{
    long long x;

    if (!PyArg_ParseTuple(args, "L", &x))
        return NULL;

    return PyLong_FromLongLong(x * 2 + 1);
}

static PyObject *sp_double_minus_one(PyObject *self, PyObject *args)
{
    long long x;

    if (!PyArg_ParseTuple(args, "L", &x))
        return NULL;

    return PyLong_FromLongLong(x * 2 - 1);
}

static PyObject *sp_triple_plus_one(PyObject *self, PyObject *args)
{
    long long x;

    if (!PyArg_ParseTuple(args, "L", &x))
        return NULL;

    return PyLong_FromLongLong(x * 3 + 1);
}

static PyObject *sp_triple_minus_one(PyObject *self, PyObject *args)
{
    long long x;

    if (!PyArg_ParseTuple(args, "L", &x))
        return NULL;

    return PyLong_FromLongLong(x * 3 - 1);
}

static PyObject *sp_successor(PyObject *self, PyObject *args)
{
    long long x;

    if (!PyArg_ParseTuple(args, "L", &x))
        return NULL;

    return PyLong_FromLongLong(x + 1);
}

static PyObject *sp_predecessor(PyObject *self, PyObject *args)
{
    long long x;

    if (!PyArg_ParseTuple(args, "L", &x))
        return NULL;

    return PyLong_FromLongLong(x - 1);
}

static PyObject *sp_negate_if_positive(PyObject *self, PyObject *args)
{
    long long x;

    if (!PyArg_ParseTuple(args, "L", &x))
        return NULL;

    if (x > 0)
        x = -x;

    return PyLong_FromLongLong(x);
}

static PyObject *sp_negate_if_negative(PyObject *self, PyObject *args)
{
    long long x;

    if (!PyArg_ParseTuple(args, "L", &x))
        return NULL;

    if (x < 0)
        x = -x;

    return PyLong_FromLongLong(x);
}

static PyObject *sp_zero_if_negative(PyObject *self, PyObject *args)
{
    long long x;

    if (!PyArg_ParseTuple(args, "L", &x))
        return NULL;

    if (x < 0)
        x = 0;

    return PyLong_FromLongLong(x);
}

static PyObject *sp_zero_if_positive(PyObject *self, PyObject *args)
{
    long long x;

    if (!PyArg_ParseTuple(args, "L", &x))
        return NULL;

    if (x > 0)
        x = 0;

    return PyLong_FromLongLong(x);
}

static PyObject *sp_positive_part(PyObject *self, PyObject *args)
{
    long long x;

    if (!PyArg_ParseTuple(args, "L", &x))
        return NULL;

    if (x < 0)
        x = 0;

    return PyLong_FromLongLong(x);
}

static PyObject *sp_negative_part(PyObject *self, PyObject *args)
{
    long long x;

    if (!PyArg_ParseTuple(args, "L", &x))
        return NULL;

    if (x > 0)
        x = 0;

    return PyLong_FromLongLong(x);
}

static PyObject *sp_absolute_difference_three(PyObject *self, PyObject *args)
{
    long long a, b, c;
    long long result;

    if (!PyArg_ParseTuple(args, "LLL", &a, &b, &c))
        return NULL;

    result = a - b - c;

    if (result < 0)
        result = -result;

    return PyLong_FromLongLong(result);
}

static PyObject *sp_linear_expression(PyObject *self, PyObject *args)
{
    long long a, x, b;

    if (!PyArg_ParseTuple(args, "LLL", &a, &x, &b))
        return NULL;

    return PyLong_FromLongLong(a * x + b);
}

static PyObject *sp_scale(PyObject *self, PyObject *args)
{
    long long value, factor;

    if (!PyArg_ParseTuple(args, "LL", &value, &factor))
        return NULL;

    return PyLong_FromLongLong(value * factor);
}

static PyObject *sp_offset(PyObject *self, PyObject *args)
{
    long long value, amount;

    if (!PyArg_ParseTuple(args, "LL", &value, &amount))
        return NULL;

    return PyLong_FromLongLong(value + amount);
}

static PyObject *sp_percentage_change(PyObject *self, PyObject *args)
{
    long long old_value, new_value;
    double result;

    if (!PyArg_ParseTuple(args, "LL", &old_value, &new_value))
        return NULL;

    if (old_value == 0) {
        PyErr_SetString(
            PyExc_ZeroDivisionError,
            "old value cannot be zero"
        );
        return NULL;
    }

    result = (
        ((double)new_value - (double)old_value) /
        (double)old_value
    ) * 100.0;

    return PyFloat_FromDouble(result);
}

static PyObject *sp_fraction_of(PyObject *self, PyObject *args)
{
    long long numerator, denominator;

    if (!PyArg_ParseTuple(args, "LL", &numerator, &denominator))
        return NULL;

    if (denominator == 0) {
        PyErr_SetString(
            PyExc_ZeroDivisionError,
            "denominator cannot be zero"
        );
        return NULL;
    }

    return PyFloat_FromDouble(
        (double)numerator / (double)denominator
    );
}

static PyObject *sp_reciprocal_float(PyObject *self, PyObject *args)
{
    double x;

    if (!PyArg_ParseTuple(args, "d", &x))
        return NULL;

    if (x == 0.0) {
        PyErr_SetString(
            PyExc_ZeroDivisionError,
            "reciprocal of zero"
        );
        return NULL;
    }

    return PyFloat_FromDouble(1.0 / x);
}

static PyObject *sp_scale_float(PyObject *self, PyObject *args)
{
    double value, factor;

    if (!PyArg_ParseTuple(args, "dd", &value, &factor))
        return NULL;

    return PyFloat_FromDouble(value * factor);
}

static PyObject *sp_add_float(PyObject *self, PyObject *args)
{
    double a, b;

    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    return PyFloat_FromDouble(a + b);
}

static PyObject *sp_subtract_float(PyObject *self, PyObject *args)
{
    double a, b;

    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    return PyFloat_FromDouble(a - b);
}

static PyObject *sp_multiply_float(PyObject *self, PyObject *args)
{
    double a, b;

    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    return PyFloat_FromDouble(a * b);
}

static PyObject *sp_divide_float(PyObject *self, PyObject *args)
{
    double a, b;

    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;

    if (b == 0.0) {
        PyErr_SetString(
            PyExc_ZeroDivisionError,
            "division by zero"
        );
        return NULL;
    }

    return PyFloat_FromDouble(a / b);
}

static PyObject *sp_square_float(PyObject *self, PyObject *args)
{
    double x;

    if (!PyArg_ParseTuple(args, "d", &x))
        return NULL;

    return PyFloat_FromDouble(x * x);
}

static PyObject *sp_cube_float(PyObject *self, PyObject *args)
{
    double x;

    if (!PyArg_ParseTuple(args, "d", &x))
        return NULL;

    return PyFloat_FromDouble(x * x * x);
}

static PyObject *sp_percent_float(PyObject *self, PyObject *args)
{
    double percent, value;

    if (!PyArg_ParseTuple(args, "dd", &percent, &value))
        return NULL;

    return PyFloat_FromDouble(
        percent * value / 100.0
    );
}

static PyMethodDef arithmetic_methods[] = {
    {"add", sp_add, METH_VARARGS, "Add two integers."},
    {"subtract", sp_subtract, METH_VARARGS, "Subtract two integers."},
    {"multiply", sp_multiply, METH_VARARGS, "Multiply two integers."},
    {"power", sp_power, METH_VARARGS, "Integer power."},
    {"square", sp_square, METH_VARARGS, "Square an integer."},
    {"cube", sp_cube, METH_VARARGS, "Cube an integer."},
    {"negate", sp_negate, METH_VARARGS, "Negate an integer."},
    {"absolute", sp_absolute, METH_VARARGS, "Absolute value."},
    {"increment", sp_increment, METH_VARARGS, "Increment by one."},
    {"decrement", sp_decrement, METH_VARARGS, "Decrement by one."},
    {"double", sp_double, METH_VARARGS, "Double a value."},
    {"triple", sp_triple, METH_VARARGS, "Triple a value."},
    {"quadruple", sp_quadruple, METH_VARARGS, "Multiply by four."},
    {"half", sp_half, METH_VARARGS, "Integer half."},
    {"third", sp_third, METH_VARARGS, "Integer third."},
    {"remainder", sp_remainder, METH_VARARGS, "Integer remainder."},
    {"floor_divide", sp_floor_divide, METH_VARARGS, "Floor division."},
    {"is_even", sp_is_even, METH_VARARGS, "Test whether even."},
    {"is_odd", sp_is_odd, METH_VARARGS, "Test whether odd."},
    {"sign", sp_sign, METH_VARARGS, "Return sign."},
    {"min", sp_min, METH_VARARGS, "Minimum of two values."},
    {"max", sp_max, METH_VARARGS, "Maximum of two values."},
    {"clamp", sp_clamp, METH_VARARGS, "Clamp a value."},
    {"difference", sp_difference, METH_VARARGS, "Absolute difference."},
    {"sum_two", sp_sum_two, METH_VARARGS, "Sum two values."},
    {"product_two", sp_product_two, METH_VARARGS, "Product of two values."},
    {"average_two", sp_average_two, METH_VARARGS, "Average of two values."},
    {"midpoint", sp_midpoint, METH_VARARGS, "Midpoint of two values."},
    {"percent_of", sp_percent_of, METH_VARARGS, "Percentage of a value."},
    {"percentage", sp_percentage, METH_VARARGS, "Calculate percentage."},
    {"add_percent", sp_add_percent, METH_VARARGS, "Add a percentage."},
    {"subtract_percent", sp_subtract_percent, METH_VARARGS, "Subtract a percentage."},
    {"ratio", sp_ratio, METH_VARARGS, "Calculate ratio."},
    {"reciprocal", sp_reciprocal, METH_VARARGS, "Calculate reciprocal."},
    {"is_positive", sp_is_positive, METH_VARARGS, "Test positive."},
    {"is_negative", sp_is_negative, METH_VARARGS, "Test negative."},
    {"is_zero", sp_is_zero, METH_VARARGS, "Test zero."},
    {"abs_difference", sp_abs_difference, METH_VARARGS, "Absolute difference."},
    {"increment_by", sp_increment_by, METH_VARARGS, "Increment by amount."},
    {"decrement_by", sp_decrement_by, METH_VARARGS, "Decrement by amount."},
    {"add_three", sp_add_three, METH_VARARGS, "Add three integers."},
    {"sum_range", sp_sum_range, METH_VARARGS, "Sum an integer range."},
    {"count_range", sp_count_range, METH_VARARGS, "Count."},
    {"distance", sp_distance, METH_VARARGS, "Distance between two values."},
    {"nearest_multiple", sp_nearest_multiple, METH_VARARGS, "Nearest multiple."},
    {"round_down_multiple", sp_round_down_multiple, METH_VARARGS, "Round down to a multiple."},
    {"round_up_multiple", sp_round_up_multiple, METH_VARARGS, "Round up to a multiple."},
    {"mod_positive", sp_mod_positive, METH_VARARGS, "Positive modulo."},
    {"is_multiple", sp_is_multiple, METH_VARARGS, "Test whether a value is a multiple."},
    {"is_divisible_by", sp_is_divisible_by, METH_VARARGS, "Test divisibility."},
    {"divide_exact", sp_divide_exact, METH_VARARGS, "Exact integer division."},
    {"greater", sp_greater, METH_VARARGS, "Greater-than comparison."},
    {"less", sp_less, METH_VARARGS, "Less-than comparison."},
    {"equal", sp_equal, METH_VARARGS, "Equality comparison."},
    {"not_equal", sp_not_equal, METH_VARARGS, "Inequality comparison."},
    {"greater_equal", sp_greater_equal, METH_VARARGS, "Greater-or-equal comparison."},
    {"less_equal", sp_less_equal, METH_VARARGS, "Less-or-equal comparison."},
    {"between", sp_between, METH_VARARGS, "Test whether a value is between bounds."},
    {"is_integer_value", sp_is_integer_value, METH_VARARGS, "Test integer value."},
    {"power_of_two", sp_power_of_two, METH_VARARGS, "Calculate a power of two."},
    {"times_ten", sp_times_ten, METH_VARARGS, "Multiply by ten."},
    {"times_hundred", sp_times_hundred, METH_VARARGS, "Multiply by one hundred."},
    {"times_thousand", sp_times_thousand, METH_VARARGS, "Multiply by one thousand."},
    {"add_ten", sp_add_ten, METH_VARARGS, "Add ten."},
    {"subtract_ten", sp_subtract_ten, METH_VARARGS, "Subtract ten."},
    {"add_hundred", sp_add_hundred, METH_VARARGS, "Add one hundred."},
    {"subtract_hundred", sp_subtract_hundred, METH_VARARGS, "Subtract one hundred."},
    {"add_thousand", sp_add_thousand, METH_VARARGS, "Add one thousand."},
    {"subtract_thousand", sp_subtract_thousand, METH_VARARGS, "Subtract one thousand."},
    {"multiply_by_zero", sp_multiply_by_zero, METH_VARARGS, "Multiply by zero."},
    {"multiply_by_one", sp_multiply_by_one, METH_VARARGS, "Multiply by one."},
    {"multiply_by_five", sp_multiply_by_five, METH_VARARGS, "Multiply by five."},
    {"multiply_by_ten", sp_multiply_by_ten, METH_VARARGS, "Multiply by ten."},
    {"multiply_by_hundred", sp_multiply_by_hundred, METH_VARARGS, "Multiply by one hundred."},
    {"divide_by_one", sp_divide_by_one, METH_VARARGS, "Divide by one."},
    {"divide_by_two", sp_divide_by_two, METH_VARARGS, "Divide by two."},
    {"divide_by_five", sp_divide_by_five, METH_VARARGS, "Divide by five."},
    {"divide_by_ten", sp_divide_by_ten, METH_VARARGS, "Divide by ten."},
    {"divide_by_hundred", sp_divide_by_hundred, METH_VARARGS, "Divide by one hundred."},
    {"remainder_positive", sp_remainder_positive, METH_VARARGS, "Positive remainder."},
    {"quotient", sp_quotient, METH_VARARGS, "Integer quotient."},
    {"divmod_quotient", sp_divmod_quotient, METH_VARARGS, "Division quotient."},
    {"divmod_remainder", sp_divmod_remainder, METH_VARARGS, "Division remainder."},
    {"average_three", sp_average_three, METH_VARARGS, "Average of three values."},
    {"average_four", sp_average_four, METH_VARARGS, "Average of four values."},
    {"sum_three", sp_sum_three, METH_VARARGS, "Sum three values."},
    {"sum_four", sp_sum_four, METH_VARARGS, "Sum four values."},
    {"product_three", sp_product_three, METH_VARARGS, "Product of three values."},
    {"product_four", sp_product_four, METH_VARARGS, "Product of four values."},
    {"min_three", sp_min_three, METH_VARARGS, "Minimum of three values."},
    {"max_three", sp_max_three, METH_VARARGS, "Maximum of three values."},
    {"min_four", sp_min_four, METH_VARARGS, "Minimum of four values."},
    {"max_four", sp_max_four, METH_VARARGS, "Maximum of four values."},
    {"range_sum_formula", sp_range_sum_formula, METH_VARARGS, "Range sum formula."},
    {"triangular_number", sp_triangular_number, METH_VARARGS, "Triangular number."},
    {"double_factorial_simple", sp_double_factorial_simple, METH_VARARGS, "Double factorial."},
    {"square_plus_one", sp_square_plus_one, METH_VARARGS, "Square plus one."},
    {"square_minus_one", sp_square_minus_one, METH_VARARGS, "Square minus one."},
    {"cube_plus_one", sp_cube_plus_one, METH_VARARGS, "Cube plus one."},
    {"cube_minus_one", sp_cube_minus_one, METH_VARARGS, "Cube minus one."},
    {"double_plus_one", sp_double_plus_one, METH_VARARGS, "Double plus one."},
    {"double_minus_one", sp_double_minus_one, METH_VARARGS, "Double minus one."},
    {"triple_plus_one", sp_triple_plus_one, METH_VARARGS, "Triple plus one."},
    {"triple_minus_one", sp_triple_minus_one, METH_VARARGS, "Triple minus one."},
    {"successor", sp_successor, METH_VARARGS, "Successor."},
    {"predecessor", sp_predecessor, METH_VARARGS, "Predecessor."},
    {"negate_if_positive", sp_negate_if_positive, METH_VARARGS, "Negate positive values."},
    {"negate_if_negative", sp_negate_if_negative, METH_VARARGS, "Negate negative values."},
    {"zero_if_negative", sp_zero_if_negative, METH_VARARGS, "Replace negative values with zero."},
    {"zero_if_positive", sp_zero_if_positive, METH_VARARGS, "Replace positive values with zero."},
    {"positive_part", sp_positive_part, METH_VARARGS, "Positive part."},
    {"negative_part", sp_negative_part, METH_VARARGS, "Negative part."},
    {"absolute_difference_three", sp_absolute_difference_three, METH_VARARGS, "Absolute three-value difference."},
    {"linear_expression", sp_linear_expression, METH_VARARGS, "Evaluate ax + b."},
    {"scale", sp_scale, METH_VARARGS, "Scale an integer."},
    {"offset", sp_offset, METH_VARARGS, "Offset an integer."},
    {"percentage_change", sp_percentage_change, METH_VARARGS, "Percentage change."},
    {"fraction_of", sp_fraction_of, METH_VARARGS, "Calculate a fraction."},
    {"reciprocal_float", sp_reciprocal_float, METH_VARARGS, "Floating-point reciprocal."},
    {"scale_float", sp_scale_float, METH_VARARGS, "Scale a floating-point value."},
    {"add_float", sp_add_float, METH_VARARGS, "Add floating-point values."},
    {"subtract_float", sp_subtract_float, METH_VARARGS, "Subtract floating-point values."},
    {"multiply_float", sp_multiply_float, METH_VARARGS, "Multiply floating-point values."},
    {"divide_float", sp_divide_float, METH_VARARGS, "Divide floating-point values."},
    {"square_float", sp_square_float, METH_VARARGS, "Square a floating-point value."},
    {"cube_float", sp_cube_float, METH_VARARGS, "Cube a floating-point value."},
    {"percent_float", sp_percent_float, METH_VARARGS, "Calculate floating-point percentage."},

    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef arithmetic_module = {
    PyModuleDef_HEAD_INIT,
    "arithmetic",
    "High-performance arithmetic operations for speedpy.",
    -1,
    arithmetic_methods
};

PyMODINIT_FUNC PyInit_arithmetic(void)
{
    return PyModule_Create(&arithmetic_module);
}
