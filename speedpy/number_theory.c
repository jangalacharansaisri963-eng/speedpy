#include <Python.h>

static PyObject *sp_nt_gcd(PyObject *self, PyObject *args)
{
    long long a, b;
    long long t;

    if (!PyArg_ParseTuple(args, "LL", &a, &b))
        return NULL;

    if (a < 0)
        a = -a;

    if (b < 0)
        b = -b;

    while (b != 0) {
        t = a % b;
        a = b;
        b = t;
    }

    return PyLong_FromLongLong(a);
}

static PyObject *sp_nt_lcm(PyObject *self, PyObject *args)
{
    long long a, b;
    long long x, y;
    long long gcd;
    long long result;

    if (!PyArg_ParseTuple(args, "LL", &a, &b))
        return NULL;

    if (a == 0 || b == 0)
        return PyLong_FromLongLong(0);

    x = a < 0 ? -a : a;
    y = b < 0 ? -b : b;

    gcd = x;
    b = y;

    while (b != 0) {
        long long t = gcd % b;
        gcd = b;
        b = t;
    }

    result = (x / gcd) * y;

    return PyLong_FromLongLong(result);
}

static PyObject *sp_nt_is_divisible(PyObject *self, PyObject *args)
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

static PyObject *sp_nt_is_prime(PyObject *self, PyObject *args)
{
    long long n;
    long long i;

    if (!PyArg_ParseTuple(args, "L", &n))
        return NULL;

    if (n < 2)
        Py_RETURN_FALSE;

    if (n == 2)
        Py_RETURN_TRUE;

    if ((n & 1) == 0)
        Py_RETURN_FALSE;

    for (i = 3; i <= n / i; i += 2) {
        if (n % i == 0)
            Py_RETURN_FALSE;
    }

    Py_RETURN_TRUE;
}

static PyObject *sp_nt_is_composite(PyObject *self, PyObject *args)
{
    long long n;
    long long i;

    if (!PyArg_ParseTuple(args, "L", &n))
        return NULL;

    if (n < 4)
        Py_RETURN_FALSE;

    if ((n & 1) == 0)
        Py_RETURN_TRUE;

    for (i = 3; i <= n / i; i += 2) {
        if (n % i == 0)
            Py_RETURN_TRUE;
    }

    Py_RETURN_FALSE;
}

static PyObject *sp_nt_is_prime_or_one(PyObject *self, PyObject *args)
{
    long long n;
    long long i;

    if (!PyArg_ParseTuple(args, "L", &n))
        return NULL;

    if (n == 1)
        Py_RETURN_TRUE;

    if (n < 2)
        Py_RETURN_FALSE;

    if (n == 2)
        Py_RETURN_TRUE;

    if ((n & 1) == 0)
        Py_RETURN_FALSE;

    for (i = 3; i <= n / i; i += 2) {
        if (n % i == 0)
            Py_RETURN_FALSE;
    }

    Py_RETURN_TRUE;
}

static PyObject *sp_nt_next_prime(PyObject *self, PyObject *args)
{
    long long n;
    long long candidate;
    long long i;
    int prime;

    if (!PyArg_ParseTuple(args, "L", &n))
        return NULL;

    if (n < 2)
        return PyLong_FromLongLong(2);

    candidate = n + 1;

    if (candidate == 3)
        return PyLong_FromLongLong(3);

    if ((candidate & 1) == 0)
        candidate++;

    for (;;) {
        prime = 1;

        for (i = 3; i <= candidate / i; i += 2) {
            if (candidate % i == 0) {
                prime = 0;
                break;
            }
        }

        if (prime)
            return PyLong_FromLongLong(candidate);

        candidate += 2;
    }
}

static PyObject *sp_nt_previous_prime(PyObject *self, PyObject *args)
{
    long long n;
    long long candidate;
    long long i;
    int prime;

    if (!PyArg_ParseTuple(args, "L", &n))
        return NULL;

    if (n <= 2) {
        PyErr_SetString(
            PyExc_ValueError,
            "there is no prime below 2"
        );
        return NULL;
    }

    candidate = n - 1;

    if (candidate == 2)
        return PyLong_FromLongLong(2);

    if ((candidate & 1) == 0)
        candidate--;

    for (;;) {
        prime = 1;

        for (i = 3; i <= candidate / i; i += 2) {
            if (candidate % i == 0) {
                prime = 0;
                break;
            }
        }

        if (prime)
            return PyLong_FromLongLong(candidate);

        candidate -= 2;
    }
}

static PyObject *sp_nt_prime_count(PyObject *self, PyObject *args)
{
    long long n;
    long long count = 0;
    long long i;
    long long j;
    int prime;

    if (!PyArg_ParseTuple(args, "L", &n))
        return NULL;

    if (n < 2)
        return PyLong_FromLongLong(0);

    count = 1;

    for (i = 3; i <= n; i += 2) {
        prime = 1;

        for (j = 3; j <= i / j; j += 2) {
            if (i % j == 0) {
                prime = 0;
                break;
            }
        }

        if (prime)
            count++;
    }

    return PyLong_FromLongLong(count);
}

static PyObject *sp_nt_smallest_prime_factor(PyObject *self, PyObject *args)
{
    long long n;
    long long i;

    if (!PyArg_ParseTuple(args, "L", &n))
        return NULL;

    if (n < 2) {
        PyErr_SetString(
            PyExc_ValueError,
            "n must be at least 2"
        );
        return NULL;
    }

    if ((n & 1) == 0)
        return PyLong_FromLongLong(2);

    for (i = 3; i <= n / i; i += 2) {
        if (n % i == 0)
            return PyLong_FromLongLong(i);
    }

    return PyLong_FromLongLong(n);
}

static PyObject *sp_nt_largest_prime_factor(PyObject *self, PyObject *args)
{
    long long n;
    long long factor = 2;

    if (!PyArg_ParseTuple(args, "L", &n))
        return NULL;

    if (n < 2) {
        PyErr_SetString(
            PyExc_ValueError,
            "n must be at least 2"
        );
        return NULL;
    }

    while (n % 2 == 0) {
        factor = 2;
        n /= 2;
    }

    while (n > 1) {
        long long i;
        int found = 0;

        for (i = 3; i <= n / i; i += 2) {
            if (n % i == 0) {
                factor = i;
                n /= i;
                found = 1;
                break;
            }
        }

        if (!found) {
            factor = n;
            break;
        }
    }

    return PyLong_FromLongLong(factor);
}

static PyObject *sp_nt_factor_count(PyObject *self, PyObject *args)
{
    long long n;
    long long count = 0;

    if (!PyArg_ParseTuple(args, "L", &n))
        return NULL;

    if (n < 1) {
        PyErr_SetString(
            PyExc_ValueError,
            "n must be positive"
        );
        return NULL;
    }

    while (n % 2 == 0) {
        count++;
        n /= 2;
    }

    {
        long long p;

        for (p = 3; p <= n / p; p += 2) {
            while (n % p == 0) {
                count++;
                n /= p;
            }
        }
    }

    if (n > 1)
        count++;

    return PyLong_FromLongLong(count);
}

static PyObject *sp_nt_distinct_prime_factor_count(PyObject *self, PyObject *args)
{
    long long n;
    long long count = 0;
    long long p;

    if (!PyArg_ParseTuple(args, "L", &n))
        return NULL;

    if (n < 1) {
        PyErr_SetString(
            PyExc_ValueError,
            "n must be positive"
        );
        return NULL;
    }

    if (n % 2 == 0) {
        count++;

        while (n % 2 == 0)
            n /= 2;
    }

    for (p = 3; p <= n / p; p += 2) {
        if (n % p == 0) {
            count++;

            while (n % p == 0)
                n /= p;
        }
    }

    if (n > 1)
        count++;

    return PyLong_FromLongLong(count);
}

static PyObject *sp_nt_divisor_count(PyObject *self, PyObject *args)
{
    long long n;
    long long count = 0;
    long long i;

    if (!PyArg_ParseTuple(args, "L", &n))
        return NULL;

    if (n <= 0) {
        PyErr_SetString(
            PyExc_ValueError,
            "n must be positive"
        );
        return NULL;
    }

    for (i = 1; i <= n / i; i++) {
        if (n % i == 0) {
            if (i == n / i)
                count++;
            else
                count += 2;
        }
    }

    return PyLong_FromLongLong(count);
}

static PyObject *sp_nt_proper_divisor_count(PyObject *self, PyObject *args)
{
    long long n;
    long long count = 0;
    long long i;

    if (!PyArg_ParseTuple(args, "L", &n))
        return NULL;

    if (n <= 0) {
        PyErr_SetString(
            PyExc_ValueError,
            "n must be positive"
        );
        return NULL;
    }

    if (n == 1)
        return PyLong_FromLongLong(0);

    for (i = 1; i <= n / i; i++) {
        if (n % i == 0) {
            if (i != n)
                count++;

            if (i != 1 && i != n / i)
                count++;
        }
    }

    return PyLong_FromLongLong(count);
}

static PyObject *sp_nt_divisor_sum(PyObject *self, PyObject *args)
{
    long long n;
    long long sum = 0;
    long long i;

    if (!PyArg_ParseTuple(args, "L", &n))
        return NULL;

    if (n <= 0) {
        PyErr_SetString(
            PyExc_ValueError,
            "n must be positive"
        );
        return NULL;
    }

    for (i = 1; i <= n / i; i++) {
        if (n % i == 0) {
            sum += i;

            if (i != n / i)
                sum += n / i;
        }
    }

    return PyLong_FromLongLong(sum);
}

static PyObject *sp_nt_proper_divisor_sum(PyObject *self, PyObject *args)
{
    long long n;
    long long sum = 0;
    long long i;

    if (!PyArg_ParseTuple(args, "L", &n))
        return NULL;

    if (n <= 0) {
        PyErr_SetString(
            PyExc_ValueError,
            "n must be positive"
        );
        return NULL;
    }

    if (n == 1)
        return PyLong_FromLongLong(0);

    for (i = 1; i <= n / i; i++) {
        if (n % i == 0) {
            if (i != n)
                sum += i;

            if (i != 1 && i != n / i)
                sum += n / i;
        }
    }

    return PyLong_FromLongLong(sum);
}

static PyObject *sp_nt_is_perfect(PyObject *self, PyObject *args)
{
    long long n;
    long long sum = 1;
    long long i;

    if (!PyArg_ParseTuple(args, "L", &n))
        return NULL;

    if (n <= 1)
        Py_RETURN_FALSE;

    for (i = 2; i <= n / i; i++) {
        if (n % i == 0) {
            sum += i;

            if (i != n / i)
                sum += n / i;
        }
    }

    if (sum == n)
        Py_RETURN_TRUE;

    Py_RETURN_FALSE;
}

static PyObject *sp_nt_is_abundant(PyObject *self, PyObject *args)
{
    long long n;
    long long sum = 1;
    long long i;

    if (!PyArg_ParseTuple(args, "L", &n))
        return NULL;

    if (n < 12)
        Py_RETURN_FALSE;

    for (i = 2; i <= n / i; i++) {
        if (n % i == 0) {
            sum += i;

            if (i != n / i)
                sum += n / i;
        }
    }

    if (sum > n)
        Py_RETURN_TRUE;

    Py_RETURN_FALSE;
}

static PyObject *sp_nt_is_deficient(PyObject *self, PyObject *args)
{
    long long n;
    long long sum = 1;
    long long i;

    if (!PyArg_ParseTuple(args, "L", &n))
        return NULL;

    if (n <= 0)
        Py_RETURN_FALSE;

    if (n == 1)
        Py_RETURN_TRUE;

    for (i = 2; i <= n / i; i++) {
        if (n % i == 0) {
            sum += i;

            if (i != n / i)
                sum += n / i;
        }
    }

    if (sum < n)
        Py_RETURN_TRUE;

    Py_RETURN_FALSE;
}

static PyObject *sp_nt_is_square(PyObject *self, PyObject *args)
{
    long long n;
    long long low;
    long long high;
    long long root = 0;

    if (!PyArg_ParseTuple(args, "L", &n))
        return NULL;

    if (n < 0)
        Py_RETURN_FALSE;

    low = 0;
    high = n < 3037000499LL ? n : 3037000499LL;

    while (low <= high) {
        long long mid = low + (high - low) / 2;

        if (mid == 0 || mid <= n / mid) {
            root = mid;
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }

    return PyBool_FromLong(root * root == n);
}

static PyObject *sp_nt_is_cube(PyObject *self, PyObject *args)
{
    long long n;
    long long low;
    long long high = 2097151;
    long long root = 0;

    if (!PyArg_ParseTuple(args, "L", &n))
        return NULL;

    if (n < 0)
        Py_RETURN_FALSE;

    low = 0;

    if (n < high)
        high = n;

    while (low <= high) {
        long long mid = low + (high - low) / 2;

        if (mid == 0 || mid <= n / mid / mid) {
            root = mid;
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }

    return PyBool_FromLong(root * root * root == n);
}

static PyObject *sp_nt_is_power_of_two(PyObject *self, PyObject *args)
{
    long long n;

    if (!PyArg_ParseTuple(args, "L", &n))
        return NULL;

    if (n > 0 && (n & (n - 1)) == 0)
        Py_RETURN_TRUE;

    Py_RETURN_FALSE;
}

static PyObject *sp_nt_power_of_two_exponent(PyObject *self, PyObject *args)
{
    long long n;
    long long exponent = 0;

    if (!PyArg_ParseTuple(args, "L", &n))
        return NULL;

    if (n <= 0 || (n & (n - 1)) != 0) {
        PyErr_SetString(
            PyExc_ValueError,
            "n must be a positive power of two"
        );
        return NULL;
    }

    while (n > 1) {
        n >>= 1;
        exponent++;
    }

    return PyLong_FromLongLong(exponent);
}

static PyObject *sp_nt_modulo(PyObject *self, PyObject *args)
{
    long long a, m;
    long long result;

    if (!PyArg_ParseTuple(args, "LL", &a, &m))
        return NULL;

    if (m == 0) {
        PyErr_SetString(
            PyExc_ZeroDivisionError,
            "modulus cannot be zero"
        );
        return NULL;
    }

    result = a % m;

    if (result != 0 && ((result < 0) != (m < 0)))
        result += m;

    return PyLong_FromLongLong(result);
}

static PyObject *sp_nt_mod_power(PyObject *self, PyObject *args)
{
    long long base, exponent, modulus;
    long long result;

    if (!PyArg_ParseTuple(args, "LLL", &base, &exponent, &modulus))
        return NULL;

    if (modulus <= 0) {
        PyErr_SetString(
            PyExc_ValueError,
            "modulus must be positive"
        );
        return NULL;
    }

    if (exponent < 0) {
        PyErr_SetString(
            PyExc_ValueError,
            "exponent must be non-negative"
        );
        return NULL;
    }

    base %= modulus;
    if (base < 0)
        base += modulus;

    result = 1 % modulus;

    while (exponent > 0) {
        if (exponent & 1)
            result = (result * base) % modulus;

        base = (base * base) % modulus;
        exponent >>= 1;
    }

    return PyLong_FromLongLong(result);
}

static PyObject *sp_nt_extended_gcd(PyObject *self, PyObject *args)
{
    long long a, b;
    long long old_r, r;
    long long old_s, s;
    long long old_t, t;
    long long q;
    PyObject *result;

    if (!PyArg_ParseTuple(args, "LL", &a, &b))
        return NULL;

    old_r = a;
    r = b;
    old_s = 1;
    s = 0;
    old_t = 0;
    t = 1;

    while (r != 0) {
        q = old_r / r;

        {
            long long temp = old_r - q * r;
            old_r = r;
            r = temp;
        }

        {
            long long temp = old_s - q * s;
            old_s = s;
            s = temp;
        }

        {
            long long temp = old_t - q * t;
            old_t = t;
            t = temp;
        }
    }

    if (old_r < 0) {
        old_r = -old_r;
        old_s = -old_s;
        old_t = -old_t;
    }

    result = PyTuple_New(3);

    if (result == NULL)
        return NULL;

    PyTuple_SET_ITEM(
        result,
        0,
        PyLong_FromLongLong(old_r)
    );

    PyTuple_SET_ITEM(
        result,
        1,
        PyLong_FromLongLong(old_s)
    );

    PyTuple_SET_ITEM(
        result,
        2,
        PyLong_FromLongLong(old_t)
    );

    return result;
}

static PyObject *sp_nt_mod_inverse(PyObject *self, PyObject *args)
{
    long long a, modulus;
    long long old_r, r;
    long long old_s, s;
    long long q;
    long long inverse;

    if (!PyArg_ParseTuple(args, "LL", &a, &modulus))
        return NULL;

    if (modulus <= 0) {
        PyErr_SetString(
            PyExc_ValueError,
            "modulus must be positive"
        );
        return NULL;
    }

    a %= modulus;
    if (a < 0)
        a += modulus;

    old_r = a;
    r = modulus;
    old_s = 1;
    s = 0;

    while (r != 0) {
        q = old_r / r;

        {
            long long temp = old_r - q * r;
            old_r = r;
            r = temp;
        }

        {
            long long temp = old_s - q * s;
            old_s = s;
            s = temp;
        }
    }

    if (old_r != 1) {
        PyErr_SetString(
            PyExc_ValueError,
            "modular inverse does not exist"
        );
        return NULL;
    }

    inverse = old_s % modulus;

    if (inverse < 0)
        inverse += modulus;

    return PyLong_FromLongLong(inverse);
}

static PyObject *sp_nt_totient(PyObject *self, PyObject *args)
{
    long long n;
    long long result;
    long long p;

    if (!PyArg_ParseTuple(args, "L", &n))
        return NULL;

    if (n <= 0) {
        PyErr_SetString(
            PyExc_ValueError,
            "n must be positive"
        );
        return NULL;
    }

    result = n;

    if (n % 2 == 0) {
        result -= result / 2;

        while (n % 2 == 0)
            n /= 2;
    }

    for (p = 3; p <= n / p; p += 2) {
        if (n % p == 0) {
            result -= result / p;

            while (n % p == 0)
                n /= p;
        }
    }

    if (n > 1)
        result -= result / n;

    return PyLong_FromLongLong(result);
}

static PyObject *sp_nt_coprime(PyObject *self, PyObject *args)
{
    long long a, b;
    long long t;

    if (!PyArg_ParseTuple(args, "LL", &a, &b))
        return NULL;

    if (a < 0)
        a = -a;

    if (b < 0)
        b = -b;

    while (b != 0) {
        t = a % b;
        a = b;
        b = t;
    }

    if (a == 1)
        Py_RETURN_TRUE;

    Py_RETURN_FALSE;
}

static PyObject *sp_nt_mobius(PyObject *self, PyObject *args)
{
    long long n;
    long long p;
    int factors = 0;

    if (!PyArg_ParseTuple(args, "L", &n))
        return NULL;

    if (n <= 0) {
        PyErr_SetString(
            PyExc_ValueError,
            "n must be positive"
        );
        return NULL;
    }

    if (n % 2 == 0) {
        n /= 2;
        factors++;

        if (n % 2 == 0)
            return PyLong_FromLongLong(0);
    }

    for (p = 3; p <= n / p; p += 2) {
        if (n % p == 0) {
            n /= p;
            factors++;

            if (n % p == 0)
                return PyLong_FromLongLong(0);
        }
    }

    if (n > 1)
        factors++;

    if (factors & 1)
        return PyLong_FromLongLong(-1);

    return PyLong_FromLongLong(1);
}

static PyObject *sp_nt_carmichael(PyObject *self, PyObject *args)
{
    long long n;
    long long result;
    long long p;
    long long power;

    if (!PyArg_ParseTuple(args, "L", &n))
        return NULL;

    if (n <= 0) {
        PyErr_SetString(
            PyExc_ValueError,
            "n must be positive"
        );
        return NULL;
    }

    if (n == 1)
        return PyLong_FromLongLong(1);

    result = 1;

    if (n % 2 == 0) {
        power = 1;

        while (n % 2 == 0) {
            n /= 2;
            power *= 2;
        }

        if (power == 4)
            result = 2;
        else if (power > 4)
            result = power / 2;
    }

    for (p = 3; p <= n / p; p += 2) {
        if (n % p == 0) {
            long long prime_power = p;

            while (n % p == 0) {
                n /= p;
                prime_power *= p;
            }

            prime_power /= p;

            {
                long long local = prime_power - prime_power / p;
                long long a = result;
                long long b = local;

                while (b != 0) {
                    long long t = a % b;
                    a = b;
                    b = t;
                }

                result = (result / a) * local;
            }
        }
    }

    if (n > 1) {
        long long local = n - 1;
        long long a = result;
        long long b = local;

        while (b != 0) {
            long long t = a % b;
            a = b;
            b = t;
        }

        result = (result / a) * local;
    }

    return PyLong_FromLongLong(result);
}

static PyObject *sp_nt_euclidean_distance_index(PyObject *self, PyObject *args)
{
    long long a, b;
    long long result = 0;

    if (!PyArg_ParseTuple(args, "LL", &a, &b))
        return NULL;

    if (a < 0)
        a = -a;

    if (b < 0)
        b = -b;

    while (a != 0 && b != 0) {
        if (a > b)
            a -= b;
        else
            b -= a;

        result++;
    }

    return PyLong_FromLongLong(result);
}

static PyObject *sp_nt_fibonacci(PyObject *self, PyObject *args)
{
    long long n;
    long long a = 0;
    long long b = 1;
    long long i;

    if (!PyArg_ParseTuple(args, "L", &n))
        return NULL;

    if (n < 0) {
        PyErr_SetString(
            PyExc_ValueError,
            "n must be non-negative"
        );
        return NULL;
    }

    for (i = 0; i < n; i++) {
        long long next = a + b;
        a = b;
        b = next;
    }

    return PyLong_FromLongLong(a);
}

static PyObject *sp_nt_fibonacci_fast(PyObject *self, PyObject *args)
{
    long long n;
    long long a = 0;
    long long b = 1;
    long long bit;

    if (!PyArg_ParseTuple(args, "L", &n))
        return NULL;

    if (n < 0) {
        PyErr_SetString(
            PyExc_ValueError,
            "n must be non-negative"
        );
        return NULL;
    }

    bit = 1;

    while (bit <= n / 2)
        bit <<= 1;

    while (bit > 0) {
        long long c;
        long long d;

        c = a * (2 * b - a);
        d = a * a + b * b;

        if (n & bit) {
            a = d;
            b = c + d;
        } else {
            a = c;
            b = d;
        }

        bit >>= 1;
    }

    return PyLong_FromLongLong(a);
}

static PyObject *sp_nt_fibonacci_is_member(PyObject *self, PyObject *args)
{
    long long n;
    long long a = 0;
    long long b = 1;

    if (!PyArg_ParseTuple(args, "L", &n))
        return NULL;

    if (n < 0)
        Py_RETURN_FALSE;

    while (b < n) {
        long long next = a + b;
        a = b;
        b = next;
    }

    if (a == n || b == n)
        Py_RETURN_TRUE;

    Py_RETURN_FALSE;
}

static PyObject *sp_nt_lucas(PyObject *self, PyObject *args)
{
    long long n;
    long long a = 2;
    long long b = 1;
    long long i;

    if (!PyArg_ParseTuple(args, "L", &n))
        return NULL;

    if (n < 0) {
        PyErr_SetString(
            PyExc_ValueError,
            "n must be non-negative"
        );
        return NULL;
    }

    for (i = 0; i < n; i++) {
        long long next = a + b;
        a = b;
        b = next;
    }

    return PyLong_FromLongLong(a);
}

static PyObject *sp_nt_triangular(PyObject *self, PyObject *args)
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

    return PyLong_FromLongLong(
        n * (n + 1) / 2
    );
}

static PyObject *sp_nt_pentagonal(PyObject *self, PyObject *args)
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

    return PyLong_FromLongLong(
        n * (3 * n - 1) / 2
    );
}

static PyObject *sp_nt_hexagonal(PyObject *self, PyObject *args)
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

    return PyLong_FromLongLong(
        n * (2 * n - 1)
    );
}

static PyObject *sp_nt_heptagonal(PyObject *self, PyObject *args)
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

    return PyLong_FromLongLong(
        n * (5 * n - 3) / 2
    );
}

static PyObject *sp_nt_octagonal(PyObject *self, PyObject *args)
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

    return PyLong_FromLongLong(
        n * (3 * n - 2)
    );
}

static PyObject *sp_nt_central_polygonal(PyObject *self, PyObject *args)
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

    return PyLong_FromLongLong(
        n * n + n + 1
    );
}

static PyObject *sp_nt_catalan(PyObject *self, PyObject *args)
{
    long long n;
    long long result = 1;
    long long i;

    if (!PyArg_ParseTuple(args, "L", &n))
        return NULL;

    if (n < 0) {
        PyErr_SetString(
            PyExc_ValueError,
            "n must be non-negative"
        );
        return NULL;
    }

    for (i = 1; i <= n; i++) {
        result = result * (n + i) / i;
    }

    return PyLong_FromLongLong(
        result / (n + 1)
    );
}

static PyObject *sp_nt_fibonacci_sum(PyObject *self, PyObject *args)
{
    long long n;
    long long a = 0;
    long long b = 1;
    long long sum = 0;
    long long i;

    if (!PyArg_ParseTuple(args, "L", &n))
        return NULL;

    if (n < 0) {
        PyErr_SetString(
            PyExc_ValueError,
            "n must be non-negative"
        );
        return NULL;
    }

    for (i = 0; i <= n; i++) {
        sum += a;

        {
            long long next = a + b;
            a = b;
            b = next;
        }
    }

    return PyLong_FromLongLong(sum);
}

static PyObject *sp_nt_digit_sum(PyObject *self, PyObject *args)
{
    long long n;
    long long sum = 0;

    if (!PyArg_ParseTuple(args, "L", &n))
        return NULL;

    if (n < 0)
        n = -n;

    while (n > 0) {
        sum += n % 10;
        n /= 10;
    }

    return PyLong_FromLongLong(sum);
}

static PyObject *sp_nt_digit_product(PyObject *self, PyObject *args)
{
    long long n;
    long long product = 1;

    if (!PyArg_ParseTuple(args, "L", &n))
        return NULL;

    if (n < 0)
        n = -n;

    if (n == 0)
        return PyLong_FromLongLong(0);

    while (n > 0) {
        product *= n % 10;
        n /= 10;
    }

    return PyLong_FromLongLong(product);
}

static PyObject *sp_nt_digit_count(PyObject *self, PyObject *args)
{
    long long n;
    long long count = 0;

    if (!PyArg_ParseTuple(args, "L", &n))
        return NULL;

    if (n < 0)
        n = -n;

    if (n == 0)
        return PyLong_FromLongLong(1);

    while (n > 0) {
        count++;
        n /= 10;
    }

    return PyLong_FromLongLong(count);
}

static PyObject *sp_nt_reverse_digits(PyObject *self, PyObject *args)
{
    long long n;
    long long sign = 1;
    long long result = 0;

    if (!PyArg_ParseTuple(args, "L", &n))
        return NULL;

    if (n < 0) {
        sign = -1;
        n = -n;
    }

    while (n > 0) {
        result = result * 10 + n % 10;
        n /= 10;
    }

    return PyLong_FromLongLong(result * sign);
}

static PyObject *sp_nt_is_palindrome_number(PyObject *self, PyObject *args)
{
    long long n;
    long long original;
    long long reversed = 0;

    if (!PyArg_ParseTuple(args, "L", &n))
        return NULL;

    if (n < 0)
        Py_RETURN_FALSE;

    original = n;

    while (n > 0) {
        reversed = reversed * 10 + n % 10;
        n /= 10;
    }

    if (original == reversed)
        Py_RETURN_TRUE;

    Py_RETURN_FALSE;
}

static PyObject *sp_nt_digital_root(PyObject *self, PyObject *args)
{
    long long n;

    if (!PyArg_ParseTuple(args, "L", &n))
        return NULL;

    if (n < 0)
        n = -n;

    if (n == 0)
        return PyLong_FromLongLong(0);

    return PyLong_FromLongLong(
        1 + (n - 1) % 9
    );
}

static PyObject *sp_nt_is_armstrong_three_digit(PyObject *self, PyObject *args)
{
    long long n;
    long long original;
    long long sum = 0;

    if (!PyArg_ParseTuple(args, "L", &n))
        return NULL;

    if (n < 0 || n > 999)
        Py_RETURN_FALSE;

    original = n;

    while (n > 0) {
        long long digit = n % 10;
        sum += digit * digit * digit;
        n /= 10;
    }

    if (sum == original)
        Py_RETURN_TRUE;

    Py_RETURN_FALSE;
}

static PyObject *sp_nt_is_semiprime(PyObject *self, PyObject *args)
{
    long long n;
    long long count = 0;
    long long p = 2;

    if (!PyArg_ParseTuple(args, "L", &n))
        return NULL;

    if (n < 4)
        Py_RETURN_FALSE;

    while (p <= n / p)
    {
        while (n % p == 0)
        {
            n /= p;
            count++;

            if (count > 2)
                Py_RETURN_FALSE;
        }

        p = (p == 2) ? 3 : p + 2;
    }

    if (n > 1)
        count++;

    return PyBool_FromLong(count == 2);
}

static PyObject *sp_nt_is_sphenic(PyObject *self, PyObject *args)
{
    long long n;
    int distinct = 0;
    long long p = 2;

    if (!PyArg_ParseTuple(args, "L", &n))
        return NULL;

    if (n < 30)
        Py_RETURN_FALSE;

    while (p <= n / p)
    {
        if (n % p == 0)
        {
            int exponent = 0;

            distinct++;

            while (n % p == 0)
            {
                n /= p;
                exponent++;

                if (exponent > 1)
                    Py_RETURN_FALSE;
            }

            if (distinct > 3)
                Py_RETURN_FALSE;
        }

        p = (p == 2) ? 3 : p + 2;
    }

    if (n > 1)
        distinct++;

    return PyBool_FromLong(distinct == 3);
}

static PyObject *sp_nt_is_squarefree(PyObject *self, PyObject *args)
{
    long long n;
    long long p = 2;

    if (!PyArg_ParseTuple(args, "L", &n))
        return NULL;

    if (n == 0)
        Py_RETURN_FALSE;

    if (n < 0)
        n = -n;

    while (p <= n / p)
    {
        if (n % (p * p) == 0)
            Py_RETURN_FALSE;

        p = (p == 2) ? 3 : p + 2;
    }

    Py_RETURN_TRUE;
}

static PyObject *sp_nt_aliquot_sum(PyObject *self, PyObject *args)
{
    long long n;
    long long sum = 1;
    long long d;

    if (!PyArg_ParseTuple(args, "L", &n))
        return NULL;

    if (n <= 1)
        return PyLong_FromLongLong(0);

    for (d = 2; d <= n / d; d++)
    {
        if (n % d == 0)
        {
            sum += d;

            if (d != n / d)
                sum += n / d;
        }
    }

    return PyLong_FromLongLong(sum);
}

static PyObject *sp_nt_abundant_excess(PyObject *self, PyObject *args)
{
    long long n;
    long long sum = 0;
    long long d;

    if (!PyArg_ParseTuple(args, "L", &n))
        return NULL;

    if (n <= 0)
        return PyLong_FromLongLong(0);

    for (d = 1; d <= n / d; d++)
    {
        if (n % d == 0)
        {
            if (d != n)
                sum += d;

            if (d != 1 && d != n / d && n / d != n)
                sum += n / d;
        }
    }

    return PyLong_FromLongLong(sum - n);
}

static PyObject *sp_nt_deficiency(PyObject *self, PyObject *args)
{
    long long n;
    long long sum = 0;
    long long d;

    if (!PyArg_ParseTuple(args, "L", &n))
        return NULL;

    if (n <= 0)
        return PyLong_FromLongLong(0);

    for (d = 1; d <= n / d; d++)
    {
        if (n % d == 0)
        {
            if (d != n)
                sum += d;

            if (d != 1 && d != n / d && n / d != n)
                sum += n / d;
        }
    }

    return PyLong_FromLongLong(n - sum);
}

static PyObject *sp_nt_is_perfect_power(PyObject *self, PyObject *args)
{
    long long n;
    long long base;
    long long value;
    int exponent;

    if (!PyArg_ParseTuple(args, "L", &n))
        return NULL;

    if (n <= 1)
        Py_RETURN_FALSE;

    for (base = 2; base <= n / base; base++)
    {
        value = base;

        for (exponent = 2; value <= n / base; exponent++)
        {
            value *= base;

            if (value == n)
                Py_RETURN_TRUE;

            if (value > n / base)
                break;
        }
    }

    Py_RETURN_FALSE;
}

static PyObject *sp_nt_floor_sqrt(PyObject *self, PyObject *args)
{
    long long n;
    long long low = 0;
    long long high;
    long long answer = 0;

    if (!PyArg_ParseTuple(args, "L", &n))
        return NULL;

    if (n < 0)
    {
        PyErr_SetString(PyExc_ValueError, "square root of negative number");
        return NULL;
    }

    high = n < 3037000499LL ? n : 3037000499LL;

    while (low <= high)
    {
        long long mid = low + (high - low) / 2;

        if (mid == 0 || mid <= n / mid)
        {
            answer = mid;
            low = mid + 1;
        }
        else
        {
            high = mid - 1;
        }
    }

    return PyLong_FromLongLong(answer);
}

static PyObject *sp_nt_ceil_sqrt(PyObject *self, PyObject *args)
{
    long long n;
    long long r;
    long long square;

    if (!PyArg_ParseTuple(args, "L", &n))
        return NULL;

    if (n < 0)
    {
        PyErr_SetString(PyExc_ValueError, "square root of negative number");
        return NULL;
    }

    if (n == 0)
        return PyLong_FromLongLong(0);

    r = 0;

    while (r <= 3037000499LL && r <= n / (r + 1))
        r++;

    square = r * r;

    if (square < n)
        r++;

    return PyLong_FromLongLong(r);
}

static PyObject *sp_nt_floor_cuberoot(PyObject *self, PyObject *args)
{
    long long n;
    long long low = 0;
    long long high = 2097151;
    long long answer = 0;

    if (!PyArg_ParseTuple(args, "L", &n))
        return NULL;

    if (n < 0)
    {
        PyErr_SetString(PyExc_ValueError, "cube root of negative number");
        return NULL;
    }

    if (n < high)
        high = n;

    while (low <= high)
    {
        long long mid = low + (high - low) / 2;

        if (mid == 0 || mid <= n / mid / mid)
        {
            answer = mid;
            low = mid + 1;
        }
        else
        {
            high = mid - 1;
        }
    }

    return PyLong_FromLongLong(answer);
}

static PyObject *sp_nt_ceil_cuberoot(PyObject *self, PyObject *args)
{
    long long n;
    long long r = 0;

    if (!PyArg_ParseTuple(args, "L", &n))
        return NULL;

    if (n < 0)
    {
        PyErr_SetString(PyExc_ValueError, "cube root of negative number");
        return NULL;
    }

    while (r < 2097152 && r * r * r < n)
        r++;

    return PyLong_FromLongLong(r);
}

static PyObject *sp_nt_binomial(PyObject *self, PyObject *args)
{
    long long n;
    long long k;
    long long result = 1;
    long long i;

    if (!PyArg_ParseTuple(args, "LL", &n, &k))
        return NULL;

    if (n < 0 || k < 0 || k > n)
        return PyLong_FromLongLong(0);

    if (k > n - k)
        k = n - k;

    for (i = 1; i <= k; i++)
    {
        if (result > 9223372036854775807LL / (n - k + i))
        {
            PyErr_SetString(PyExc_OverflowError, "binomial coefficient exceeds 64-bit range");
            return NULL;
        }

        result *= n - k + i;
        result /= i;
    }

    return PyLong_FromLongLong(result);
}

static PyObject *sp_nt_factorial(PyObject *self, PyObject *args)
{
    long long n;
    long long result = 1;
    long long i;

    if (!PyArg_ParseTuple(args, "L", &n))
        return NULL;

    if (n < 0)
    {
        PyErr_SetString(PyExc_ValueError, "factorial requires a non-negative integer");
        return NULL;
    }

    for (i = 2; i <= n; i++)
    {
        if (result > 9223372036854775807LL / i)
        {
            PyErr_SetString(PyExc_OverflowError, "factorial exceeds 64-bit range");
            return NULL;
        }

        result *= i;
    }

    return PyLong_FromLongLong(result);
}

static PyObject *sp_nt_double_factorial(PyObject *self, PyObject *args)
{
    long long n;
    long long result = 1;
    long long i;

    if (!PyArg_ParseTuple(args, "L", &n))
        return NULL;

    if (n < -1)
    {
        PyErr_SetString(PyExc_ValueError, "invalid double factorial");
        return NULL;
    }

    for (i = n; i > 1; i -= 2)
    {
        if (result > 9223372036854775807LL / i)
        {
            PyErr_SetString(PyExc_OverflowError, "double factorial exceeds 64-bit range");
            return NULL;
        }

        result *= i;
    }

    return PyLong_FromLongLong(result);
}

static PyObject *sp_nt_derangement(PyObject *self, PyObject *args)
{
    long long n;
    long long a = 1;
    long long b = 0;
    long long i;
    long long c;

    if (!PyArg_ParseTuple(args, "L", &n))
        return NULL;

    if (n < 0)
    {
        PyErr_SetString(PyExc_ValueError, "n must be non-negative");
        return NULL;
    }

    if (n == 0)
        return PyLong_FromLongLong(1);

    if (n == 1)
        return PyLong_FromLongLong(0);

    for (i = 2; i <= n; i++)
    {
        if (b > (9223372036854775807LL - a) / (i - 1))
        {
            PyErr_SetString(PyExc_OverflowError, "derangement exceeds 64-bit range");
            return NULL;
        }

        c = (i - 1) * (a + b);
        a = b;
        b = c;
    }

    return PyLong_FromLongLong(b);
}

static PyObject *sp_nt_bell_number(PyObject *self, PyObject *args)
{
    long long n;
    long long row[64];
    long long i;
    long long j;

    if (!PyArg_ParseTuple(args, "L", &n))
        return NULL;

    if (n < 0 || n >= 64)
    {
        PyErr_SetString(PyExc_ValueError, "n must be between 0 and 63");
        return NULL;
    }

    for (i = 0; i < 64; i++)
        row[i] = 0;

    row[0] = 1;

    for (i = 1; i <= n; i++)
    {
        long long next[64];

        for (j = 0; j < 64; j++)
            next[j] = 0;

        next[0] = row[i - 1];

        for (j = 1; j <= i; j++)
        {
            if (next[j - 1] > 9223372036854775807LL - row[j - 1])
            {
                PyErr_SetString(PyExc_OverflowError, "Bell number exceeds 64-bit range");
                return NULL;
            }

            next[j] = next[j - 1] + row[j - 1];
        }

        for (j = 0; j <= i; j++)
            row[j] = next[j];
    }

    return PyLong_FromLongLong(row[0]);
}

static PyObject *sp_nt_stirling_second(PyObject *self, PyObject *args)
{
    long long n;
    long long k;
    long long dp[64][64];
    long long i;
    long long j;

    if (!PyArg_ParseTuple(args, "LL", &n, &k))
        return NULL;

    if (n < 0 || k < 0 || n >= 64 || k >= 64)
    {
        PyErr_SetString(PyExc_ValueError, "n and k must be between 0 and 63");
        return NULL;
    }

    if (k > n)
        return PyLong_FromLongLong(0);

    for (i = 0; i < 64; i++)
        for (j = 0; j < 64; j++)
            dp[i][j] = 0;

    dp[0][0] = 1;

    for (i = 1; i <= n; i++)
    {
        for (j = 1; j <= i; j++)
        {
            if (dp[i - 1][j] > (9223372036854775807LL - dp[i - 1][j - 1]) / j)
            {
                PyErr_SetString(PyExc_OverflowError, "Stirling number exceeds 64-bit range");
                return NULL;
            }

            dp[i][j] = dp[i - 1][j - 1] + j * dp[i - 1][j];
        }
    }

    return PyLong_FromLongLong(dp[n][k]);
}

static PyObject *sp_nt_stirling_first(PyObject *self, PyObject *args)
{
    long long n;
    long long k;
    long long dp[64][64];
    long long i;
    long long j;

    if (!PyArg_ParseTuple(args, "LL", &n, &k))
        return NULL;

    if (n < 0 || k < 0 || n >= 64 || k >= 64)
    {
        PyErr_SetString(PyExc_ValueError, "n and k must be between 0 and 63");
        return NULL;
    }

    if (k > n)
        return PyLong_FromLongLong(0);

    for (i = 0; i < 64; i++)
        for (j = 0; j < 64; j++)
            dp[i][j] = 0;

    dp[0][0] = 1;

    for (i = 1; i <= n; i++)
    {
        for (j = 1; j <= i; j++)
        {
            long long value = dp[i - 1][j - 1];
            long long multiplier = i - 1;

            if (multiplier > 0)
            {
                if (dp[i - 1][j] > 9223372036854775807LL / multiplier)
                {
                    PyErr_SetString(PyExc_OverflowError, "Stirling number exceeds 64-bit range");
                    return NULL;
                }

                value += (multiplier * dp[i - 1][j]);
            }

            dp[i][j] = value;
        }
    }

    return PyLong_FromLongLong(dp[n][k]);
}

static PyObject *sp_nt_partition_number(PyObject *self, PyObject *args)
{
    long long n;
    long long dp[501];
    long long i;
    long long j;

    if (!PyArg_ParseTuple(args, "L", &n))
        return NULL;

    if (n < 0 || n > 500)
    {
        PyErr_SetString(PyExc_ValueError, "n must be between 0 and 500");
        return NULL;
    }

    for (i = 0; i <= 500; i++)
        dp[i] = 0;

    dp[0] = 1;

    for (i = 1; i <= n; i++)
    {
        for (j = i; j <= n; j++)
        {
            if (dp[j] > 9223372036854775807LL - dp[j - i])
            {
                PyErr_SetString(PyExc_OverflowError, "partition number exceeds 64-bit range");
                return NULL;
            }

            dp[j] += dp[j - i];
        }
    }

    return PyLong_FromLongLong(dp[n]);
}

static PyObject *sp_nt_primorial(PyObject *self, PyObject *args)
{
    long long n;
    long long result = 1;
    long long p;

    if (!PyArg_ParseTuple(args, "L", &n))
        return NULL;

    if (n < 2)
        return PyLong_FromLongLong(1);

    for (p = 2; p <= n;)
    {
        int prime = 1;
        long long d;

        if (p > 2 && p % 2 == 0)
            prime = 0;

        for (d = 3; prime && d <= p / d; d += 2)
        {
            if (p % d == 0)
            {
                prime = 0;
                break;
            }
        }

        if (prime)
        {
            if (result > 9223372036854775807LL / p)
            {
                PyErr_SetString(PyExc_OverflowError, "primorial exceeds 64-bit range");
                return NULL;
            }

            result *= p;
        }

        p = (p == 2) ? 3 : p + 2;
    }

    return PyLong_FromLongLong(result);
}

static PyObject *sp_nt_prime_gap(PyObject *self, PyObject *args)
{
    long long n;
    long long next;
    long long previous;

    if (!PyArg_ParseTuple(args, "L", &n))
        return NULL;

    if (n < 2)
    {
        PyErr_SetString(PyExc_ValueError, "n must be at least 2");
        return NULL;
    }

    previous = n;

    while (previous >= 2)
    {
        int prime = 1;
        long long d;

        for (d = 2; d <= previous / d; d++)
        {
            if (previous % d == 0)
            {
                prime = 0;
                break;
            }
        }

        if (prime)
            break;

        previous--;
    }

    next = n + 1;

    while (next > n)
    {
        int prime = 1;
        long long d;

        if (next >= 2)
        {
            for (d = 2; d <= next / d; d++)
            {
                if (next % d == 0)
                {
                    prime = 0;
                    break;
                }
            }

            if (prime)
                break;
        }

        next++;
    }

    return PyLong_FromLongLong(next - previous);
}

static PyObject *sp_nt_binary_digit_count(PyObject *self, PyObject *args)
{
    long long n;
    long long count = 0;

    if (!PyArg_ParseTuple(args, "L", &n))
        return NULL;

    if (n < 0)
        n = -n;

    if (n == 0)
        return PyLong_FromLongLong(1);

    while (n > 0)
    {
        count++;
        n /= 2;
    }

    return PyLong_FromLongLong(count);
}

static PyObject *sp_nt_popcount(PyObject *self, PyObject *args)
{
    unsigned long long n;
    long long count = 0;

    if (!PyArg_ParseTuple(args, "K", &n))
        return NULL;

    while (n)
    {
        n &= n - 1;
        count++;
    }

    return PyLong_FromLongLong(count);
}

static PyObject *sp_nt_hamming_weight(PyObject *self, PyObject *args)
{
    unsigned long long n;
    long long count = 0;

    if (!PyArg_ParseTuple(args, "K", &n))
        return NULL;

    while (n)
    {
        count += n & 1ULL;
        n >>= 1;
    }

    return PyLong_FromLongLong(count);
}

static PyObject *sp_nt_bit_length(PyObject *self, PyObject *args)
{
    unsigned long long n;
    long long count = 0;

    if (!PyArg_ParseTuple(args, "K", &n))
        return NULL;

    while (n)
    {
        count++;
        n >>= 1;
    }

    return PyLong_FromLongLong(count);
}

static PyObject *sp_nt_is_mersenne(PyObject *self, PyObject *args)
{
    unsigned long long n;

    if (!PyArg_ParseTuple(args, "K", &n))
        return NULL;

    if (n == 0)
        Py_RETURN_FALSE;

    return PyBool_FromLong((n & (n + 1ULL)) == 0);
}

static PyObject *sp_nt_mersenne(PyObject *self, PyObject *args)
{
    long long p;
    unsigned long long result = 1;
    long long i;

    if (!PyArg_ParseTuple(args, "L", &p))
        return NULL;

    if (p < 0 || p >= 64)
    {
        PyErr_SetString(PyExc_ValueError, "p must be between 0 and 63");
        return NULL;
    }

    for (i = 0; i < p; i++)
        result <<= 1;

    result -= 1;

    return PyLong_FromUnsignedLongLong(result);
}

static PyObject *sp_nt_fibonacci_gcd(PyObject *self, PyObject *args)
{
    long long a;
    long long b;
    long long x;
    long long y;

    if (!PyArg_ParseTuple(args, "LL", &a, &b))
        return NULL;

    if (a < 0)
        a = -a;

    if (b < 0)
        b = -b;

    x = a;
    y = b;

    while (y)
    {
        long long r = x % y;
        x = y;
        y = r;
    }

    return PyLong_FromLongLong(x);
}

static PyObject *sp_nt_lucas_gcd(PyObject *self, PyObject *args)
{
    long long a;
    long long b;
    long long x;
    long long y;

    if (!PyArg_ParseTuple(args, "LL", &a, &b))
        return NULL;

    if (a < 0)
        a = -a;

    if (b < 0)
        b = -b;

    x = a;
    y = b;

    while (y)
    {
        long long r = x % y;
        x = y;
        y = r;
    }

    return PyLong_FromLongLong(x);
}

static PyObject *sp_nt_jacobi_symbol(PyObject *self, PyObject *args)
{
    long long a;
    long long n;
    int result = 1;

    if (!PyArg_ParseTuple(args, "LL", &a, &n))
        return NULL;

    if (n <= 0 || n % 2 == 0)
    {
        PyErr_SetString(PyExc_ValueError, "n must be a positive odd integer");
        return NULL;
    }

    a %= n;
    if (a < 0)
        a += n;

    while (a != 0)
    {
        while (a % 2 == 0)
        {
            a /= 2;

            if (n % 8 == 3 || n % 8 == 5)
                result = -result;
        }

        {
            long long temp = a;
            a = n;
            n = temp;
        }

        if (a % 4 == 3 && n % 4 == 3)
            result = -result;

        a %= n;
    }

    return PyLong_FromLongLong(n == 1 ? result : 0);
}

static PyObject *sp_nt_legendre_symbol(PyObject *self, PyObject *args)
{
    long long a;
    long long p;
    long long result;

    if (!PyArg_ParseTuple(args, "LL", &a, &p))
        return NULL;

    if (p <= 2)
    {
        PyErr_SetString(PyExc_ValueError, "p must be an odd prime");
        return NULL;
    }

    if (a % p == 0)
        return PyLong_FromLongLong(0);

    result = 1;

    a %= p;
    if (a < 0)
        a += p;

    while (a)
    {
        while (a % 2 == 0)
        {
            a /= 2;

            if (p % 8 == 3 || p % 8 == 5)
                result = -result;
        }

        {
            long long temp = a;
            a = p;
            p = temp;
        }

        if (a % 4 == 3 && p % 4 == 3)
            result = -result;

        a %= p;
    }

    return PyLong_FromLongLong(p == 1 ? result : 0);
}

static PyObject *sp_nt_mod_add(PyObject *self, PyObject *args)
{
    long long a;
    long long b;
    long long m;

    if (!PyArg_ParseTuple(args, "LLL", &a, &b, &m))
        return NULL;

    if (m == 0)
    {
        PyErr_SetString(PyExc_ValueError, "modulus cannot be zero");
        return NULL;
    }

    {
        long long result = ((a % m) + (b % m)) % m;

        if (result != 0 && ((result < 0) != (m < 0)))
            result += m;

        return PyLong_FromLongLong(result);
    }
}

static PyObject *sp_nt_mod_subtract(PyObject *self, PyObject *args)
{
    long long a;
    long long b;
    long long m;

    if (!PyArg_ParseTuple(args, "LLL", &a, &b, &m))
        return NULL;

    if (m == 0)
    {
        PyErr_SetString(PyExc_ValueError, "modulus cannot be zero");
        return NULL;
    }

    {
        long long result = ((a % m) - (b % m)) % m;

        if (result != 0 && ((result < 0) != (m < 0)))
            result += m;

        return PyLong_FromLongLong(result);
    }
}

static PyObject *sp_nt_mod_multiply(PyObject *self, PyObject *args)
{
    long long a;
    long long b;
    long long m;

    if (!PyArg_ParseTuple(args, "LLL", &a, &b, &m))
        return NULL;

    if (m == 0)
    {
        PyErr_SetString(PyExc_ValueError, "modulus cannot be zero");
        return NULL;
        }

    {
        long long result = ((a % m) * (b % m)) % m;

        if (result != 0 && ((result < 0) != (m < 0)))
            result += m;

        return PyLong_FromLongLong(result);
    }
}

static PyObject *sp_nt_chinese_remainder_two(PyObject *self, PyObject *args)
{
    long long a1;
    long long m1;
    long long a2;
    long long m2;
    long long x;
    long long y;
    long long g;
    long long t;
    long long lcm;

    if (!PyArg_ParseTuple(args, "LLLL", &a1, &m1, &a2, &m2))
        return NULL;

    if (m1 <= 0 || m2 <= 0)
    {
        PyErr_SetString(PyExc_ValueError, "moduli must be positive");
        return NULL;
    }

    x = m1;
    y = m2;

    while (y)
    {
        long long r = x % y;
        x = y;
        y = r;
    }

    g = x;

    if ((a2 - a1) % g != 0)
    {
        PyErr_SetString(PyExc_ValueError, "no solution exists");
        return NULL;
    }

    lcm = (m1 / g) * m2;

    {
        long long m1g = m1 / g;
        long long m2g = m2 / g;
        long long diff = (a2 - a1) / g;

        x = m1g;
        y = m2g;

        {
            long long old_r = x;
            long long r = y;
            long long old_s = 1;
            long long s = 0;

            while (r)
            {
                long long q = old_r / r;
                long long temp;

                temp = old_r - q * r;
                old_r = r;
                r = temp;

                temp = old_s - q * s;
                old_s = s;
                s = temp;
            }

            t = (diff * old_s) % m2g;
        }
    }

    {
        long long result = a1 + m1 * t;

        result %= lcm;

        if (result < 0)
            result += lcm;

        return PyLong_FromLongLong(result);
    }
}

static PyObject *sp_nt_fermat_test(PyObject *self, PyObject *args)
{
    long long n;
    long long a;
    long long result = 1;
    long long base;
    long long exp;

    if (!PyArg_ParseTuple(args, "LL", &n, &a))
        return NULL;

    if (n <= 1)
        Py_RETURN_FALSE;

    a %= n;

    if (a < 0)
        a += n;

    base = a;
    exp = n - 1;

    while (exp)
    {
        if (exp & 1)
            result = (result * base) % n;

        base = (base * base) % n;
        exp >>= 1;
    }

    return PyBool_FromLong(result == 1);
}

static PyObject *sp_nt_is_harshad(PyObject *self, PyObject *args)
{
    long long n;
    long long value;
    long long sum = 0;

    if (!PyArg_ParseTuple(args, "L", &n))
        return NULL;

    if (n <= 0)
        Py_RETURN_FALSE;

    value = n;

    while (value)
    {
        sum += value % 10;
        value /= 10;
    }

    if (sum == 0)
        Py_RETURN_FALSE;

    return PyBool_FromLong(n % sum == 0);
}

static PyObject *sp_nt_count_digit(PyObject *self, PyObject *args)
{
    long long n;
    long long digit;
    long long count = 0;

    if (!PyArg_ParseTuple(args, "LL", &n, &digit))
        return NULL;

    if (digit < 0 || digit > 9)
    {
        PyErr_SetString(PyExc_ValueError, "digit must be between 0 and 9");
        return NULL;
    }

    if (n < 0)
        n = -n;

    if (n == 0)
        return PyLong_FromLongLong(digit == 0 ? 1 : 0);

    while (n)
    {
        if (n % 10 == digit)
            count++;

        n /= 10;
    }

    return PyLong_FromLongLong(count);
}

static PyObject *sp_nt_contains_digit(PyObject *self, PyObject *args)
{
    long long n;
    long long digit;

    if (!PyArg_ParseTuple(args, "LL", &n, &digit))
        return NULL;

    if (digit < 0 || digit > 9)
    {
        PyErr_SetString(PyExc_ValueError, "digit must be between 0 and 9");
        return NULL;
    }

    if (n < 0)
        n = -n;

    if (n == 0)
        return PyBool_FromLong(digit == 0);

    while (n)
    {
        if (n % 10 == digit)
            Py_RETURN_TRUE;

        n /= 10;
    }

    Py_RETURN_FALSE;
}

static PyObject *sp_nt_is_automorphic(PyObject *self, PyObject *args)
{
    long long n;
    long long square;
    long long divisor = 1;
    long long value;

    if (!PyArg_ParseTuple(args, "L", &n))
        return NULL;

    if (n < 0)
        n = -n;

    square = n * n;
    value = n;

    while (value >= 10)
    {
        divisor *= 10;
        value /= 10;
    }

    divisor *= 10;

    return PyBool_FromLong(square % divisor == n);
}

static PyObject *sp_nt_is_happy(PyObject *self, PyObject *args)
{
    long long n;
    long long slow;
    long long fast;

    if (!PyArg_ParseTuple(args, "L", &n))
        return NULL;

    if (n <= 0)
        Py_RETURN_FALSE;

    slow = n;
    fast = n;

    do
    {
        long long x = slow;
        long long sum = 0;

        while (x)
        {
            long long d = x % 10;
            sum += d * d;
            x /= 10;
        }

        slow = sum;

        x = fast;
        sum = 0;

        while (x)
        {
            long long d = x % 10;
            sum += d * d;
            x /= 10;
        }

        fast = sum;

        x = fast;
        sum = 0;

        while (x)
        {
            long long d = x % 10;
            sum += d * d;
            x /= 10;
        }

        fast = sum;
    }
    while (slow != fast);

    return PyBool_FromLong(slow == 1);
}

static PyObject *sp_nt_collatz_steps(PyObject *self, PyObject *args)
{
    unsigned long long n;
    unsigned long long steps = 0;

    if (!PyArg_ParseTuple(args, "K", &n))
        return NULL;

    if (n == 0)
    {
        PyErr_SetString(PyExc_ValueError, "n must be positive");
        return NULL;
    }

    while (n != 1)
    {
        if ((n & 1ULL) == 0)
        {
            n /= 2;
        }
        else
        {
            if (n > (18446744073709551615ULL - 1ULL) / 3ULL)
            {
                PyErr_SetString(PyExc_OverflowError, "Collatz value exceeds 64-bit range");
                return NULL;
            }

            n = 3ULL * n + 1ULL;
        }

        steps++;
    }

    return PyLong_FromUnsignedLongLong(steps);
}

static PyObject *sp_nt_collatz_max(PyObject *self, PyObject *args)
{
    unsigned long long n;
    unsigned long long maximum;
    unsigned long long steps = 0;

    if (!PyArg_ParseTuple(args, "K", &n))
        return NULL;

    if (n == 0)
    {
        PyErr_SetString(PyExc_ValueError, "n must be positive");
        return NULL;
    }

    maximum = n;

    while (n != 1)
    {
        if ((n & 1ULL) == 0)
        {
            n /= 2;
        }
        else
        {
            if (n > (18446744073709551615ULL - 1ULL) / 3ULL)
            {
                PyErr_SetString(PyExc_OverflowError, "Collatz value exceeds 64-bit range");
                return NULL;
            }

            n = 3ULL * n + 1ULL;
        }

        if (n > maximum)
            maximum = n;

        steps++;

        if (steps == 0)
            break;
    }

    return PyLong_FromUnsignedLongLong(maximum);
}

static PyObject *sp_nt_sum_proper_divisors(PyObject *self, PyObject *args)
{
    long long n;
    long long sum = 0;
    long long d;

    if (!PyArg_ParseTuple(args, "L", &n))
        return NULL;

    if (n <= 1)
        return PyLong_FromLongLong(0);

    sum = 1;

    for (d = 2; d <= n / d; d++)
    {
        if (n % d == 0)
        {
            sum += d;

            if (d != n / d)
                sum += n / d;
        }
    }

    return PyLong_FromLongLong(sum);
}

static PyMethodDef number_theory_methods[] = {
    {"gcd", sp_nt_gcd, METH_VARARGS, "Greatest common divisor."},
    {"lcm", sp_nt_lcm, METH_VARARGS, "Least common multiple."},
    {"is_divisible", sp_nt_is_divisible, METH_VARARGS, "Check divisibility."},
    {"is_prime", sp_nt_is_prime, METH_VARARGS, "Check whether a number is prime."},
    {"is_composite", sp_nt_is_composite, METH_VARARGS, "Check whether a number is composite."},
    {"is_prime_or_one", sp_nt_is_prime_or_one, METH_VARARGS, "Check whether a number is prime or one."},
    {"next_prime", sp_nt_next_prime, METH_VARARGS, "Find the next prime."},
    {"previous_prime", sp_nt_previous_prime, METH_VARARGS, "Find the previous prime."},
    {"prime_count", sp_nt_prime_count, METH_VARARGS, "Count primes up to n."},
    {"smallest_prime_factor", sp_nt_smallest_prime_factor, METH_VARARGS, "Smallest prime factor."},
    {"largest_prime_factor", sp_nt_largest_prime_factor, METH_VARARGS, "Largest prime factor."},
    {"factor_count", sp_nt_factor_count, METH_VARARGS, "Number of prime factors."},
    {"distinct_prime_factor_count", sp_nt_distinct_prime_factor_count, METH_VARARGS, "Number of distinct prime factors."},
    {"divisor_count", sp_nt_divisor_count, METH_VARARGS, "Number of divisors."},
    {"proper_divisor_count", sp_nt_proper_divisor_count, METH_VARARGS, "Number of proper divisors."},
    {"divisor_sum", sp_nt_divisor_sum, METH_VARARGS, "Sum of divisors."},
    {"proper_divisor_sum", sp_nt_proper_divisor_sum, METH_VARARGS, "Sum of proper divisors."},
    {"is_perfect", sp_nt_is_perfect, METH_VARARGS, "Check for perfect number."},
    {"is_abundant", sp_nt_is_abundant, METH_VARARGS, "Check for abundant number."},
    {"is_deficient", sp_nt_is_deficient, METH_VARARGS, "Check for deficient number."},
    {"is_square", sp_nt_is_square, METH_VARARGS, "Check for perfect square."},
    {"is_cube", sp_nt_is_cube, METH_VARARGS, "Check for perfect cube."},
    {"is_power_of_two", sp_nt_is_power_of_two, METH_VARARGS, "Check for power of two."},
    {"power_of_two_exponent", sp_nt_power_of_two_exponent, METH_VARARGS, "Exponent of a power of two."},

    {"modulo", sp_nt_modulo, METH_VARARGS, "Modulo operation."},
    {"mod_power", sp_nt_mod_power, METH_VARARGS, "Modular exponentiation."},
    {"extended_gcd", sp_nt_extended_gcd, METH_VARARGS, "Extended Euclidean algorithm."},
    {"mod_inverse", sp_nt_mod_inverse, METH_VARARGS, "Modular inverse."},
    {"totient", sp_nt_totient, METH_VARARGS, "Euler totient function."},
    {"coprime", sp_nt_coprime, METH_VARARGS, "Check coprimality."},
    {"mobius", sp_nt_mobius, METH_VARARGS, "Mobius function."},
    {"carmichael", sp_nt_carmichael, METH_VARARGS, "Carmichael function."},
    {"euclidean_distance_index", sp_nt_euclidean_distance_index, METH_VARARGS, "Euclidean distance index."},
    {"fibonacci", sp_nt_fibonacci, METH_VARARGS, "Fibonacci number."},
    {"fibonacci_fast", sp_nt_fibonacci_fast, METH_VARARGS, "Fast Fibonacci number."},
    {"fibonacci_is_member", sp_nt_fibonacci_is_member, METH_VARARGS, "Check Fibonacci membership."},
    {"lucas", sp_nt_lucas, METH_VARARGS, "Lucas number."},
    {"triangular", sp_nt_triangular, METH_VARARGS, "Triangular number."},
    {"pentagonal", sp_nt_pentagonal, METH_VARARGS, "Pentagonal number."},
    {"hexagonal", sp_nt_hexagonal, METH_VARARGS, "Hexagonal number."},
    {"heptagonal", sp_nt_heptagonal, METH_VARARGS, "Heptagonal number."},
    {"octagonal", sp_nt_octagonal, METH_VARARGS, "Octagonal number."},
    {"central_polygonal", sp_nt_central_polygonal, METH_VARARGS, "Central polygonal number."},
    {"catalan", sp_nt_catalan, METH_VARARGS, "Catalan number."},
    {"fibonacci_sum", sp_nt_fibonacci_sum, METH_VARARGS, "Sum of Fibonacci numbers."},
    {"digit_sum", sp_nt_digit_sum, METH_VARARGS, "Sum of digits."},
    {"digit_product", sp_nt_digit_product, METH_VARARGS, "Product of digits."},
    {"digit_count", sp_nt_digit_count, METH_VARARGS, "Count digits."},
    {"reverse_digits", sp_nt_reverse_digits, METH_VARARGS, "Reverse digits."},
    {"is_palindrome_number", sp_nt_is_palindrome_number, METH_VARARGS, "Check numeric palindrome."},
    {"digital_root", sp_nt_digital_root, METH_VARARGS, "Digital root."},
    {"is_armstrong_three_digit", sp_nt_is_armstrong_three_digit, METH_VARARGS, "Check three-digit Armstrong number."},

    {"is_semiprime", sp_nt_is_semiprime, METH_VARARGS, "Check semiprime."},
    {"is_sphenic", sp_nt_is_sphenic, METH_VARARGS, "Check sphenic number."},
    {"is_squarefree", sp_nt_is_squarefree, METH_VARARGS, "Check square-free number."},
    {"aliquot_sum", sp_nt_aliquot_sum, METH_VARARGS, "Aliquot sum."},
    {"abundant_excess", sp_nt_abundant_excess, METH_VARARGS, "Abundant excess."},
    {"deficiency", sp_nt_deficiency, METH_VARARGS, "Deficiency."},
    {"is_perfect_power", sp_nt_is_perfect_power, METH_VARARGS, "Check perfect power."},
    {"floor_sqrt", sp_nt_floor_sqrt, METH_VARARGS, "Floor square root."},
    {"ceil_sqrt", sp_nt_ceil_sqrt, METH_VARARGS, "Ceiling square root."},
    {"floor_cuberoot", sp_nt_floor_cuberoot, METH_VARARGS, "Floor cube root."},
    {"ceil_cuberoot", sp_nt_ceil_cuberoot, METH_VARARGS, "Ceiling cube root."},
    {"binomial", sp_nt_binomial, METH_VARARGS, "Binomial coefficient."},
    {"factorial", sp_nt_factorial, METH_VARARGS, "Factorial."},
    {"double_factorial", sp_nt_double_factorial, METH_VARARGS, "Double factorial."},
    {"derangement", sp_nt_derangement, METH_VARARGS, "Derangement number."},
    {"bell_number", sp_nt_bell_number, METH_VARARGS, "Bell number."},
    {"stirling_second", sp_nt_stirling_second, METH_VARARGS, "Stirling number of the second kind."},
    {"stirling_first", sp_nt_stirling_first, METH_VARARGS, "Stirling number of the first kind."},
    {"partition_number", sp_nt_partition_number, METH_VARARGS, "Integer partition number."},
    {"primorial", sp_nt_primorial, METH_VARARGS, "Primorial."},
    {"prime_gap", sp_nt_prime_gap, METH_VARARGS, "Gap between surrounding primes."},
    {"binary_digit_count", sp_nt_binary_digit_count, METH_VARARGS, "Number of binary digits."},
    {"popcount", sp_nt_popcount, METH_VARARGS, "Count set bits."},
    {"hamming_weight", sp_nt_hamming_weight, METH_VARARGS, "Hamming weight."},
    {"bit_length", sp_nt_bit_length, METH_VARARGS, "Bit length."},
    {"is_mersenne", sp_nt_is_mersenne, METH_VARARGS, "Check Mersenne number."},
    {"mersenne", sp_nt_mersenne, METH_VARARGS, "Generate Mersenne number."},
    {"fibonacci_gcd", sp_nt_fibonacci_gcd, METH_VARARGS, "Fibonacci GCD operation."},
    {"lucas_gcd", sp_nt_lucas_gcd, METH_VARARGS, "Lucas GCD operation."},
    {"jacobi_symbol", sp_nt_jacobi_symbol, METH_VARARGS, "Jacobi symbol."},
    {"legendre_symbol", sp_nt_legendre_symbol, METH_VARARGS, "Legendre symbol."},
    {"mod_add", sp_nt_mod_add, METH_VARARGS, "Modular addition."},
    {"mod_subtract", sp_nt_mod_subtract, METH_VARARGS, "Modular subtraction."},
    {"mod_multiply", sp_nt_mod_multiply, METH_VARARGS, "Modular multiplication."},
    {"chinese_remainder_two", sp_nt_chinese_remainder_two, METH_VARARGS, "Chinese remainder theorem for two congruences."},
    {"fermat_test", sp_nt_fermat_test, METH_VARARGS, "Fermat primality test."},
    {"is_harshad", sp_nt_is_harshad, METH_VARARGS, "Check Harshad number."},
    {"count_digit", sp_nt_count_digit, METH_VARARGS, "Count occurrences of a digit."},
    {"contains_digit", sp_nt_contains_digit, METH_VARARGS, "Check whether a number contains a digit."},
    {"is_automorphic", sp_nt_is_automorphic, METH_VARARGS, "Check automorphic number."},
    {"is_happy", sp_nt_is_happy, METH_VARARGS, "Check happy number."},
    {"collatz_steps", sp_nt_collatz_steps, METH_VARARGS, "Count Collatz steps."},
    {"collatz_max", sp_nt_collatz_max, METH_VARARGS, "Find Collatz maximum."},
    {"sum_proper_divisors", sp_nt_sum_proper_divisors, METH_VARARGS, "Sum proper divisors."},

    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef number_theory_module = {
    PyModuleDef_HEAD_INIT,
    "number_theory",
    "High-performance number theory operations for speedpy.",
    -1,
    number_theory_methods
};

PyMODINIT_FUNC PyInit_number_theory(void)
{
    return PyModule_Create(&number_theory_module);
}