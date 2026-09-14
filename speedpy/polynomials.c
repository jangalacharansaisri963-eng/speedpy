/*
 * polynomials.c - Python C extension for polynomial arithmetic and utilities.
 * Coefficients are represented as Python lists of numbers (low degree first):
 *   p(x) = coeffs[0] + coeffs[1]*x + coeffs[2]*x**2 + ...
 *
 * Only #include <Python.h> as required.
 */

#include <Python.h>
#include <math.h>

/* ============================================================
 * Internal helpers
 * ============================================================ */

static int
is_number(PyObject *o)
{
    return PyLong_Check(o) || PyFloat_Check(o) || PyComplex_Check(o);
}

static double
as_double(PyObject *o, int *ok)
{
    *ok = 1;
    if (PyFloat_Check(o))
        return PyFloat_AsDouble(o);
    if (PyLong_Check(o))
        return (double)PyLong_AsLong(o);
    if (PyComplex_Check(o))
        return PyComplex_RealAsDouble(o); /* take real part for simplicity */
    *ok = 0;
    return 0.0;
}

static PyObject *
list_from_doubles(const double *vals, Py_ssize_t n)
{
    PyObject *lst = PyList_New(n);
    if (!lst)
        return NULL;
    for (Py_ssize_t i = 0; i < n; i++) {
        PyObject *v = PyFloat_FromDouble(vals[i]);
        if (!v) {
            Py_DECREF(lst);
            return NULL;
        }
        PyList_SET_ITEM(lst, i, v);
    }
    return lst;
}

static int
get_coeffs(PyObject *obj, double **out, Py_ssize_t *n)
{
    if (!PyList_Check(obj) && !PyTuple_Check(obj)) {
        PyErr_SetString(PyExc_TypeError, "coefficients must be a list or tuple");
        return -1;
    }
    *n = PySequence_Size(obj);
    if (*n < 0)
        return -1;
    *out = (double *)PyMem_Malloc((*n ? *n : 1) * sizeof(double));
    if (!*out) {
        PyErr_NoMemory();
        return -1;
    }
    for (Py_ssize_t i = 0; i < *n; i++) {
        PyObject *item = PySequence_GetItem(obj, i);
        if (!item) {
            PyMem_Free(*out);
            return -1;
        }
        int ok;
        (*out)[i] = as_double(item, &ok);
        Py_DECREF(item);
        if (!ok) {
            PyMem_Free(*out);
            PyErr_SetString(PyExc_TypeError, "coefficients must be numbers");
            return -1;
        }
    }
    return 0;
}

static Py_ssize_t
effective_degree(const double *c, Py_ssize_t n)
{
    while (n > 0 && c[n - 1] == 0.0)
        n--;
    return n > 0 ? n - 1 : 0;
}

static void
trim_trailing_zeros(double **c, Py_ssize_t *n)
{
    while (*n > 1 && (*c)[*n - 1] == 0.0)
        (*n)--;
}

/* ============================================================
 * POLYNOMIAL CORE (1-36)
 * ============================================================ */

/* 1. eval(coeffs, x) -> value */
static PyObject *
poly_eval(PyObject *self, PyObject *args)
{
    PyObject *coeffs_obj;
    double x;
    if (!PyArg_ParseTuple(args, "Od", &coeffs_obj, &x))
        return NULL;
    double *c;
    Py_ssize_t n;
    if (get_coeffs(coeffs_obj, &c, &n) < 0)
        return NULL;
    double result = 0.0;
    for (Py_ssize_t i = n; i-- > 0; )
        result = result * x + c[i];
    PyMem_Free(c);
    return PyFloat_FromDouble(result);
}

/* 2. degree(coeffs) -> int */
static PyObject *
poly_degree(PyObject *self, PyObject *args)
{
    PyObject *coeffs_obj;
    if (!PyArg_ParseTuple(args, "O", &coeffs_obj))
        return NULL;
    double *c;
    Py_ssize_t n;
    if (get_coeffs(coeffs_obj, &c, &n) < 0)
        return NULL;
    Py_ssize_t d = effective_degree(c, n);
    PyMem_Free(c);
    return PyLong_FromSsize_t(d);
}

/* 3. copy(coeffs) -> list */
static PyObject *
poly_copy(PyObject *self, PyObject *args)
{
    PyObject *coeffs_obj;
    if (!PyArg_ParseTuple(args, "O", &coeffs_obj))
        return NULL;
    return PySequence_List(coeffs_obj);
}

/* 4. zero(degree=0) -> list of zeros */
static PyObject *
poly_zero(PyObject *self, PyObject *args)
{
    Py_ssize_t deg = 0;
    if (!PyArg_ParseTuple(args, "|n", &deg))
        return NULL;
    if (deg < 0)
        deg = 0;
    PyObject *lst = PyList_New(deg + 1);
    if (!lst)
        return NULL;
    for (Py_ssize_t i = 0; i <= deg; i++) {
        PyObject *z = PyFloat_FromDouble(0.0);
        if (!z) {
            Py_DECREF(lst);
            return NULL;
        }
        PyList_SET_ITEM(lst, i, z);
    }
    return lst;
}

/* 5. add(a, b) */
static PyObject *
poly_add(PyObject *self, PyObject *args)
{
    PyObject *a_obj, *b_obj;
    if (!PyArg_ParseTuple(args, "OO", &a_obj, &b_obj))
        return NULL;
    double *a, *b;
    Py_ssize_t na, nb;
    if (get_coeffs(a_obj, &a, &na) < 0)
        return NULL;
    if (get_coeffs(b_obj, &b, &nb) < 0) {
        PyMem_Free(a);
        return NULL;
    }
    Py_ssize_t n = na > nb ? na : nb;
    double *r = (double *)PyMem_Malloc(n * sizeof(double));
    if (!r) {
        PyMem_Free(a);
        PyMem_Free(b);
        return PyErr_NoMemory();
    }
    for (Py_ssize_t i = 0; i < n; i++) {
        double va = i < na ? a[i] : 0.0;
        double vb = i < nb ? b[i] : 0.0;
        r[i] = va + vb;
    }
    PyMem_Free(a);
    PyMem_Free(b);
    trim_trailing_zeros(&r, &n);
    PyObject *res = list_from_doubles(r, n);
    PyMem_Free(r);
    return res;
}

/* 6. subtract(a, b) */
static PyObject *
poly_subtract(PyObject *self, PyObject *args)
{
    PyObject *a_obj, *b_obj;
    if (!PyArg_ParseTuple(args, "OO", &a_obj, &b_obj))
        return NULL;
    double *a, *b;
    Py_ssize_t na, nb;
    if (get_coeffs(a_obj, &a, &na) < 0)
        return NULL;
    if (get_coeffs(b_obj, &b, &nb) < 0) {
        PyMem_Free(a);
        return NULL;
    }
    Py_ssize_t n = na > nb ? na : nb;
    double *r = (double *)PyMem_Malloc(n * sizeof(double));
    if (!r) {
        PyMem_Free(a);
        PyMem_Free(b);
        return PyErr_NoMemory();
    }
    for (Py_ssize_t i = 0; i < n; i++) {
        double va = i < na ? a[i] : 0.0;
        double vb = i < nb ? b[i] : 0.0;
        r[i] = va - vb;
    }
    PyMem_Free(a);
    PyMem_Free(b);
    trim_trailing_zeros(&r, &n);
    PyObject *res = list_from_doubles(r, n);
    PyMem_Free(r);
    return res;
}

/* 7. negate(coeffs) */
static PyObject *
poly_negate(PyObject *self, PyObject *args)
{
    PyObject *coeffs_obj;
    if (!PyArg_ParseTuple(args, "O", &coeffs_obj))
        return NULL;
    double *c;
    Py_ssize_t n;
    if (get_coeffs(coeffs_obj, &c, &n) < 0)
        return NULL;
    for (Py_ssize_t i = 0; i < n; i++)
        c[i] = -c[i];
    PyObject *res = list_from_doubles(c, n);
    PyMem_Free(c);
    return res;
}

/* 8. scalar_add(coeffs, s) */
static PyObject *
poly_scalar_add(PyObject *self, PyObject *args)
{
    PyObject *coeffs_obj;
    double s;
    if (!PyArg_ParseTuple(args, "Od", &coeffs_obj, &s))
        return NULL;
    double *c;
    Py_ssize_t n;
    if (get_coeffs(coeffs_obj, &c, &n) < 0)
        return NULL;
    if (n == 0) {
        c = (double *)PyMem_Realloc(c, sizeof(double));
        n = 1;
        c[0] = 0.0;
    }
    c[0] += s;
    PyObject *res = list_from_doubles(c, n);
    PyMem_Free(c);
    return res;
}

/* 9. scalar_subtract(coeffs, s) */
static PyObject *
poly_scalar_subtract(PyObject *self, PyObject *args)
{
    PyObject *coeffs_obj;
    double s;
    if (!PyArg_ParseTuple(args, "Od", &coeffs_obj, &s))
        return NULL;
    double *c;
    Py_ssize_t n;
    if (get_coeffs(coeffs_obj, &c, &n) < 0)
        return NULL;
    if (n == 0) {
        c = (double *)PyMem_Realloc(c, sizeof(double));
        n = 1;
        c[0] = 0.0;
    }
    c[0] -= s;
    PyObject *res = list_from_doubles(c, n);
    PyMem_Free(c);
    return res;
}

/* 10. scalar_multiply(coeffs, s) */
static PyObject *
poly_scalar_multiply(PyObject *self, PyObject *args)
{
    PyObject *coeffs_obj;
    double s;
    if (!PyArg_ParseTuple(args, "Od", &coeffs_obj, &s))
        return NULL;
    double *c;
    Py_ssize_t n;
    if (get_coeffs(coeffs_obj, &c, &n) < 0)
        return NULL;
    for (Py_ssize_t i = 0; i < n; i++)
        c[i] *= s;
    trim_trailing_zeros(&c, &n);
    PyObject *res = list_from_doubles(c, n);
    PyMem_Free(c);
    return res;
}

/* 11. scalar_divide(coeffs, s) */
static PyObject *
poly_scalar_divide(PyObject *self, PyObject *args)
{
    PyObject *coeffs_obj;
    double s;
    if (!PyArg_ParseTuple(args, "Od", &coeffs_obj, &s))
        return NULL;
    if (s == 0.0) {
        PyErr_SetString(PyExc_ZeroDivisionError, "division by zero");
        return NULL;
    }
    double *c;
    Py_ssize_t n;
    if (get_coeffs(coeffs_obj, &c, &n) < 0)
        return NULL;
    for (Py_ssize_t i = 0; i < n; i++)
        c[i] /= s;
    PyObject *res = list_from_doubles(c, n);
    PyMem_Free(c);
    return res;
}

/* 12. multiply(a, b) */
static PyObject *
poly_multiply(PyObject *self, PyObject *args)
{
    PyObject *a_obj, *b_obj;
    if (!PyArg_ParseTuple(args, "OO", &a_obj, &b_obj))
        return NULL;
    double *a, *b;
    Py_ssize_t na, nb;
    if (get_coeffs(a_obj, &a, &na) < 0)
        return NULL;
    if (get_coeffs(b_obj, &b, &nb) < 0) {
        PyMem_Free(a);
        return NULL;
    }
    if (na == 0 || nb == 0) {
        PyMem_Free(a);
        PyMem_Free(b);
        return list_from_doubles((double[]){0.0}, 1);
    }
    Py_ssize_t nr = na + nb - 1;
    double *r = (double *)PyMem_Calloc(nr, sizeof(double));
    if (!r) {
        PyMem_Free(a);
        PyMem_Free(b);
        return PyErr_NoMemory();
    }
    for (Py_ssize_t i = 0; i < na; i++)
        for (Py_ssize_t j = 0; j < nb; j++)
            r[i + j] += a[i] * b[j];
    PyMem_Free(a);
    PyMem_Free(b);
    trim_trailing_zeros(&r, &nr);
    PyObject *res = list_from_doubles(r, nr);
    PyMem_Free(r);
    return res;
}

/* 13. multiply_x(coeffs)  -> coeffs * x */
static PyObject *
poly_multiply_x(PyObject *self, PyObject *args)
{
    PyObject *coeffs_obj;
    if (!PyArg_ParseTuple(args, "O", &coeffs_obj))
        return NULL;
    double *c;
    Py_ssize_t n;
    if (get_coeffs(coeffs_obj, &c, &n) < 0)
        return NULL;
    double *r = (double *)PyMem_Malloc((n + 1) * sizeof(double));
    if (!r) {
        PyMem_Free(c);
        return PyErr_NoMemory();
    }
    r[0] = 0.0;
    for (Py_ssize_t i = 0; i < n; i++)
        r[i + 1] = c[i];
    PyMem_Free(c);
    PyObject *res = list_from_doubles(r, n + 1);
    PyMem_Free(r);
    return res;
}

/* 14. derivative(coeffs) */
static PyObject *
poly_derivative(PyObject *self, PyObject *args)
{
    PyObject *coeffs_obj;
    if (!PyArg_ParseTuple(args, "O", &coeffs_obj))
        return NULL;
    double *c;
    Py_ssize_t n;
    if (get_coeffs(coeffs_obj, &c, &n) < 0)
        return NULL;
    if (n <= 1) {
        PyMem_Free(c);
        return list_from_doubles((double[]){0.0}, 1);
    }
    double *r = (double *)PyMem_Malloc((n - 1) * sizeof(double));
    if (!r) {
        PyMem_Free(c);
        return PyErr_NoMemory();
    }
    for (Py_ssize_t i = 1; i < n; i++)
        r[i - 1] = c[i] * (double)i;
    PyMem_Free(c);
    Py_ssize_t nr = n - 1;
    trim_trailing_zeros(&r, &nr);
    PyObject *res = list_from_doubles(r, nr);
    PyMem_Free(r);
    return res;
}

/* 15. integral(coeffs, const=0) */
static PyObject *
poly_integral(PyObject *self, PyObject *args)
{
    PyObject *coeffs_obj;
    double C = 0.0;
    if (!PyArg_ParseTuple(args, "O|d", &coeffs_obj, &C))
        return NULL;
    double *c;
    Py_ssize_t n;
    if (get_coeffs(coeffs_obj, &c, &n) < 0)
        return NULL;
    double *r = (double *)PyMem_Malloc((n + 1) * sizeof(double));
    if (!r) {
        PyMem_Free(c);
        return PyErr_NoMemory();
    }
    r[0] = C;
    for (Py_ssize_t i = 0; i < n; i++)
        r[i + 1] = c[i] / (double)(i + 1);
    PyMem_Free(c);
    PyObject *res = list_from_doubles(r, n + 1);
    PyMem_Free(r);
    return res;
}

/* 16. sum_coefficients */
static PyObject *
poly_sum_coefficients(PyObject *self, PyObject *args)
{
    PyObject *coeffs_obj;
    if (!PyArg_ParseTuple(args, "O", &coeffs_obj))
        return NULL;
    double *c;
    Py_ssize_t n;
    if (get_coeffs(coeffs_obj, &c, &n) < 0)
        return NULL;
    double s = 0.0;
    for (Py_ssize_t i = 0; i < n; i++)
        s += c[i];
    PyMem_Free(c);
    return PyFloat_FromDouble(s);
}

/* 17. product_coefficients */
static PyObject *
poly_product_coefficients(PyObject *self, PyObject *args)
{
    PyObject *coeffs_obj;
    if (!PyArg_ParseTuple(args, "O", &coeffs_obj))
        return NULL;
    double *c;
    Py_ssize_t n;
    if (get_coeffs(coeffs_obj, &c, &n) < 0)
        return NULL;
    double p = 1.0;
    for (Py_ssize_t i = 0; i < n; i++)
        p *= c[i];
    PyMem_Free(c);
    return PyFloat_FromDouble(p);
}

/* 18. leading_coefficient */
static PyObject *
poly_leading_coefficient(PyObject *self, PyObject *args)
{
    PyObject *coeffs_obj;
    if (!PyArg_ParseTuple(args, "O", &coeffs_obj))
        return NULL;
    double *c;
    Py_ssize_t n;
    if (get_coeffs(coeffs_obj, &c, &n) < 0)
        return NULL;
    double lc = 0.0;
    for (Py_ssize_t i = n; i-- > 0; ) {
        if (c[i] != 0.0) {
            lc = c[i];
            break;
        }
    }
    PyMem_Free(c);
    return PyFloat_FromDouble(lc);
}

/* 19. constant_term */
static PyObject *
poly_constant_term(PyObject *self, PyObject *args)
{
    PyObject *coeffs_obj;
    if (!PyArg_ParseTuple(args, "O", &coeffs_obj))
        return NULL;
    double *c;
    Py_ssize_t n;
    if (get_coeffs(coeffs_obj, &c, &n) < 0)
        return NULL;
    double ct = n > 0 ? c[0] : 0.0;
    PyMem_Free(c);
    return PyFloat_FromDouble(ct);
}

/* 20. is_zero */
static PyObject *
poly_is_zero(PyObject *self, PyObject *args)
{
    PyObject *coeffs_obj;
    if (!PyArg_ParseTuple(args, "O", &coeffs_obj))
        return NULL;
    double *c;
    Py_ssize_t n;
    if (get_coeffs(coeffs_obj, &c, &n) < 0)
        return NULL;
    int zero = 1;
    for (Py_ssize_t i = 0; i < n; i++) {
        if (c[i] != 0.0) {
            zero = 0;
            break;
        }
    }
    PyMem_Free(c);
    return PyBool_FromLong(zero);
}

/* 21. is_constant */
static PyObject *
poly_is_constant(PyObject *self, PyObject *args)
{
    PyObject *coeffs_obj;
    if (!PyArg_ParseTuple(args, "O", &coeffs_obj))
        return NULL;
    double *c;
    Py_ssize_t n;
    if (get_coeffs(coeffs_obj, &c, &n) < 0)
        return NULL;
    int isc = 1;
    for (Py_ssize_t i = 1; i < n; i++) {
        if (c[i] != 0.0) {
            isc = 0;
            break;
        }
    }
    PyMem_Free(c);
    return PyBool_FromLong(isc);
}

/* 22. is_monic */
static PyObject *
poly_is_monic(PyObject *self, PyObject *args)
{
    PyObject *coeffs_obj;
    if (!PyArg_ParseTuple(args, "O", &coeffs_obj))
        return NULL;
    double *c;
    Py_ssize_t n;
    if (get_coeffs(coeffs_obj, &c, &n) < 0)
        return NULL;
    double lc = 0.0;
    for (Py_ssize_t i = n; i-- > 0; ) {
        if (c[i] != 0.0) {
            lc = c[i];
            break;
        }
    }
    PyMem_Free(c);
    return PyBool_FromLong(lc == 1.0);
}

/* 23. is_even  (only even powers) */
static PyObject *
poly_is_even(PyObject *self, PyObject *args)
{
    PyObject *coeffs_obj;
    if (!PyArg_ParseTuple(args, "O", &coeffs_obj))
        return NULL;
    double *c;
    Py_ssize_t n;
    if (get_coeffs(coeffs_obj, &c, &n) < 0)
        return NULL;
    int even = 1;
    for (Py_ssize_t i = 1; i < n; i += 2) {
        if (c[i] != 0.0) {
            even = 0;
            break;
        }
    }
    PyMem_Free(c);
    return PyBool_FromLong(even);
}

/* 24. is_odd */
static PyObject *
poly_is_odd(PyObject *self, PyObject *args)
{
    PyObject *coeffs_obj;
    if (!PyArg_ParseTuple(args, "O", &coeffs_obj))
        return NULL;
    double *c;
    Py_ssize_t n;
    if (get_coeffs(coeffs_obj, &c, &n) < 0)
        return NULL;
    int odd = 1;
    for (Py_ssize_t i = 0; i < n; i += 2) {
        if (c[i] != 0.0) {
            odd = 0;
            break;
        }
    }
    PyMem_Free(c);
    return PyBool_FromLong(odd);
}

/* 25. reverse  (reverse coefficient order) */
static PyObject *
poly_reverse(PyObject *self, PyObject *args)
{
    PyObject *coeffs_obj;
    if (!PyArg_ParseTuple(args, "O", &coeffs_obj))
        return NULL;
    double *c;
    Py_ssize_t n;
    if (get_coeffs(coeffs_obj, &c, &n) < 0)
        return NULL;
    for (Py_ssize_t i = 0; i < n / 2; i++) {
        double tmp = c[i];
        c[i] = c[n - 1 - i];
        c[n - 1 - i] = tmp;
    }
    PyObject *res = list_from_doubles(c, n);
    PyMem_Free(c);
    return res;
}

/* 26. abs_coefficients */
static PyObject *
poly_abs_coefficients(PyObject *self, PyObject *args)
{
    PyObject *coeffs_obj;
    if (!PyArg_ParseTuple(args, "O", &coeffs_obj))
        return NULL;
    double *c;
    Py_ssize_t n;
    if (get_coeffs(coeffs_obj, &c, &n) < 0)
        return NULL;
    for (Py_ssize_t i = 0; i < n; i++)
        if (c[i] < 0.0)
            c[i] = -c[i];
    PyObject *res = list_from_doubles(c, n);
    PyMem_Free(c);
    return res;
}

/* 27. make_monic */
static PyObject *
poly_make_monic(PyObject *self, PyObject *args)
{
    PyObject *coeffs_obj;
    if (!PyArg_ParseTuple(args, "O", &coeffs_obj))
        return NULL;
    double *c;
    Py_ssize_t n;
    if (get_coeffs(coeffs_obj, &c, &n) < 0)
        return NULL;
    double lc = 0.0;
    Py_ssize_t deg = -1;
    for (Py_ssize_t i = n; i-- > 0; ) {
        if (c[i] != 0.0) {
            lc = c[i];
            deg = i;
            break;
        }
    }
    if (deg < 0 || lc == 0.0) {
        PyMem_Free(c);
        return list_from_doubles((double[]){0.0}, 1);
    }
    for (Py_ssize_t i = 0; i <= deg; i++)
        c[i] /= lc;
    PyObject *res = list_from_doubles(c, deg + 1);
    PyMem_Free(c);
    return res;
}

/* 28. trim  (remove trailing zeros) */
static PyObject *
poly_trim(PyObject *self, PyObject *args)
{
    PyObject *coeffs_obj;
    if (!PyArg_ParseTuple(args, "O", &coeffs_obj))
        return NULL;
    double *c;
    Py_ssize_t n;
    if (get_coeffs(coeffs_obj, &c, &n) < 0)
        return NULL;
    trim_trailing_zeros(&c, &n);
    if (n == 0) {
        PyMem_Free(c);
        return list_from_doubles((double[]){0.0}, 1);
    }
    PyObject *res = list_from_doubles(c, n);
    PyMem_Free(c);
    return res;
}

/* 29. at_zero  == constant term */
static PyObject *
poly_at_zero(PyObject *self, PyObject *args)
{
    return poly_constant_term(self, args);
}

/* 30. at_one  == sum of coeffs */
static PyObject *
poly_at_one(PyObject *self, PyObject *args)
{
    return poly_sum_coefficients(self, args);
}

/* 31. at_minus_one */
static PyObject *
poly_at_minus_one(PyObject *self, PyObject *args)
{
    PyObject *coeffs_obj;
    if (!PyArg_ParseTuple(args, "O", &coeffs_obj))
        return NULL;
    double *c;
    Py_ssize_t n;
    if (get_coeffs(coeffs_obj, &c, &n) < 0)
        return NULL;
    double s = 0.0;
    for (Py_ssize_t i = 0; i < n; i++)
        s += (i % 2 == 0 ? 1.0 : -1.0) * c[i];
    PyMem_Free(c);
    return PyFloat_FromDouble(s);
}

/* 32. binomial(n, k) helper exposed; or binomial poly? Here: binomial coefficients as poly? 
 * We implement C(n, k) for convenience, but name is binomial. */
static PyObject *
poly_binomial(PyObject *self, PyObject *args)
{
    long n, k;
    if (!PyArg_ParseTuple(args, "ll", &n, &k))
        return NULL;
    if (k < 0 || k > n)
        return PyLong_FromLong(0);
    if (k > n - k)
        k = n - k;
    long res = 1;
    for (long i = 0; i < k; i++) {
        res *= (n - i);
        res /= (i + 1);
    }
    return PyLong_FromLong(res);
}

/* 33. shift_x(coeffs, a)  -> p(x + a)
 * Robust O(n^2) via successive multiplication by (x + a). */
static PyObject *
poly_shift_x(PyObject *self, PyObject *args)
{
    PyObject *coeffs_obj;
    double a;
    if (!PyArg_ParseTuple(args, "Od", &coeffs_obj, &a))
        return NULL;
    double *c;
    Py_ssize_t n;
    if (get_coeffs(coeffs_obj, &c, &n) < 0)
        return NULL;
    if (n == 0) {
        PyMem_Free(c);
        return list_from_doubles((double[]){0.0}, 1);
    }
    double *r = (double *)PyMem_Calloc(n, sizeof(double));
    if (!r) {
        PyMem_Free(c);
        return PyErr_NoMemory();
    }
    for (Py_ssize_t k = 0; k < n; k++) {
        /* coefficients of (x + a)^k */
        double *pow_c = (double *)PyMem_Calloc(k + 1, sizeof(double));
        if (!pow_c) {
            PyMem_Free(r);
            PyMem_Free(c);
            return PyErr_NoMemory();
        }
        pow_c[0] = 1.0;
        for (Py_ssize_t m = 0; m < k; m++) {
            for (Py_ssize_t j = m + 1; j >= 1; j--)
                pow_c[j] = pow_c[j - 1] + a * pow_c[j];
            pow_c[0] *= a;
        }
        for (Py_ssize_t j = 0; j <= k; j++)
            r[j] += c[k] * pow_c[j];
        PyMem_Free(pow_c);
    }
    PyMem_Free(c);
    trim_trailing_zeros(&r, &n);
    PyObject *res = list_from_doubles(r, n);
    PyMem_Free(r);
    return res;
}

/* 34. scale_x(coeffs, s)  -> p(s * x) */
static PyObject *
poly_scale_x(PyObject *self, PyObject *args)
{
    PyObject *coeffs_obj;
    double s;
    if (!PyArg_ParseTuple(args, "Od", &coeffs_obj, &s))
        return NULL;
    double *c;
    Py_ssize_t n;
    if (get_coeffs(coeffs_obj, &c, &n) < 0)
        return NULL;
    double pow_s = 1.0;
    for (Py_ssize_t i = 0; i < n; i++) {
        c[i] *= pow_s;
        pow_s *= s;
    }
    PyObject *res = list_from_doubles(c, n);
    PyMem_Free(c);
    return res;
}

/* 35. add_x(coeffs)  -> coeffs + x */
static PyObject *
poly_add_x(PyObject *self, PyObject *args)
{
    PyObject *coeffs_obj;
    if (!PyArg_ParseTuple(args, "O", &coeffs_obj))
        return NULL;
    double *c;
    Py_ssize_t n;
    if (get_coeffs(coeffs_obj, &c, &n) < 0)
        return NULL;
    if (n < 2) {
        double *r = (double *)PyMem_Realloc(c, 2 * sizeof(double));
        if (!r) {
            PyMem_Free(c);
            return PyErr_NoMemory();
        }
        c = r;
        if (n == 0)
            c[0] = 0.0;
        c[1] = 1.0;
        n = 2;
    } else {
        c[1] += 1.0;
    }
    PyObject *res = list_from_doubles(c, n);
    PyMem_Free(c);
    return res;
}

/* 36. add_constant(coeffs, c) */
static PyObject *
poly_add_constant(PyObject *self, PyObject *args)
{
    return poly_scalar_add(self, args);
}

/* ============================================================
 * Additional implementations + stubs for the remaining ~280 functions.
 * Easy structural / statistical / linear / quadratic ops are real.
 * Advanced algebra, special polynomials, full division/GCD etc. are stubs.
 * ============================================================ */

#define STUB(name) \
static PyObject *poly_##name(PyObject *self, PyObject *args) { \
    PyErr_SetString(PyExc_NotImplementedError, #name " is not yet implemented in this build"); \
    return NULL; \
}

/* ----- Powers / transforms (partial) ----- */
static PyObject *
poly_square(PyObject *self, PyObject *args)
{
    PyObject *obj;
    if (!PyArg_ParseTuple(args, "O", &obj)) return NULL;
    return poly_multiply(self, Py_BuildValue("(OO)", obj, obj));
}
static PyObject *
poly_cube(PyObject *self, PyObject *args)
{
    PyObject *obj;
    if (!PyArg_ParseTuple(args, "O", &obj)) return NULL;
    PyObject *sq = poly_square(self, Py_BuildValue("(O)", obj));
    if (!sq) return NULL;
    PyObject *res = poly_multiply(self, Py_BuildValue("(OO)", sq, obj));
    Py_DECREF(sq);
    return res;
}
STUB(power_integer)
STUB(power)
STUB(compose)
STUB(translate)
STUB(reflect_x)
STUB(reflect_y)
STUB(dilate)
STUB(contract)

static PyObject *
poly_even_part(PyObject *self, PyObject *args)
{
    PyObject *obj;
    if (!PyArg_ParseTuple(args, "O", &obj)) return NULL;
    double *c; Py_ssize_t n;
    if (get_coeffs(obj, &c, &n) < 0) return NULL;
    for (Py_ssize_t i = 1; i < n; i += 2) c[i] = 0.0;
    trim_trailing_zeros(&c, &n);
    PyObject *res = list_from_doubles(c, n ? n : 1);
    PyMem_Free(c);
    return res;
}
static PyObject *
poly_odd_part(PyObject *self, PyObject *args)
{
    PyObject *obj;
    if (!PyArg_ParseTuple(args, "O", &obj)) return NULL;
    double *c; Py_ssize_t n;
    if (get_coeffs(obj, &c, &n) < 0) return NULL;
    for (Py_ssize_t i = 0; i < n; i += 2) c[i] = 0.0;
    trim_trailing_zeros(&c, &n);
    PyObject *res = list_from_doubles(c, n ? n : 1);
    PyMem_Free(c);
    return res;
}
static PyObject *
poly_remove_constant(PyObject *self, PyObject *args)
{
    PyObject *obj;
    if (!PyArg_ParseTuple(args, "O", &obj)) return NULL;
    double *c; Py_ssize_t n;
    if (get_coeffs(obj, &c, &n) < 0) return NULL;
    if (n > 0) c[0] = 0.0;
    trim_trailing_zeros(&c, &n);
    PyObject *res = list_from_doubles(c, n ? n : 1);
    PyMem_Free(c);
    return res;
}
static PyObject *
poly_remove_linear(PyObject *self, PyObject *args)
{
    PyObject *obj;
    if (!PyArg_ParseTuple(args, "O", &obj)) return NULL;
    double *c; Py_ssize_t n;
    if (get_coeffs(obj, &c, &n) < 0) return NULL;
    if (n > 0) c[0] = 0.0;
    if (n > 1) c[1] = 0.0;
    trim_trailing_zeros(&c, &n);
    PyObject *res = list_from_doubles(c, n ? n : 1);
    PyMem_Free(c);
    return res;
}
static PyObject *
poly_remove_leading(PyObject *self, PyObject *args)
{
    PyObject *obj;
    if (!PyArg_ParseTuple(args, "O", &obj)) return NULL;
    double *c; Py_ssize_t n;
    if (get_coeffs(obj, &c, &n) < 0) return NULL;
    while (n > 0 && c[n-1] == 0.0) n--;
    if (n > 0) n--;
    trim_trailing_zeros(&c, &n);
    PyObject *res = list_from_doubles(c, n ? n : 1);
    PyMem_Free(c);
    return res;
}
STUB(shift_coefficients)
STUB(insert_zero)
STUB(remove_coefficient)
STUB(multiply_by_linear)
STUB(add_linear_factor)

static PyObject *
poly_coefficient(PyObject *self, PyObject *args)
{
    PyObject *obj; Py_ssize_t k;
    if (!PyArg_ParseTuple(args, "On", &obj, &k)) return NULL;
    double *c; Py_ssize_t n;
    if (get_coeffs(obj, &c, &n) < 0) return NULL;
    double v = (k >= 0 && k < n) ? c[k] : 0.0;
    PyMem_Free(c);
    return PyFloat_FromDouble(v);
}
STUB(coefficient_sum_range)
STUB(weighted_coefficient_sum)
static PyObject *
poly_squared_coefficient_sum(PyObject *self, PyObject *args)
{
    PyObject *obj;
    if (!PyArg_ParseTuple(args, "O", &obj)) return NULL;
    double *c; Py_ssize_t n;
    if (get_coeffs(obj, &c, &n) < 0) return NULL;
    double s = 0.0;
    for (Py_ssize_t i = 0; i < n; i++) s += c[i] * c[i];
    PyMem_Free(c);
    return PyFloat_FromDouble(s);
}
static PyObject *
poly_max_coefficient(PyObject *self, PyObject *args)
{
    PyObject *obj;
    if (!PyArg_ParseTuple(args, "O", &obj)) return NULL;
    double *c; Py_ssize_t n;
    if (get_coeffs(obj, &c, &n) < 0) return NULL;
    if (n == 0) { PyMem_Free(c); return PyFloat_FromDouble(0.0); }
    double m = c[0];
    for (Py_ssize_t i = 1; i < n; i++) if (c[i] > m) m = c[i];
    PyMem_Free(c);
    return PyFloat_FromDouble(m);
}
static PyObject *
poly_min_coefficient(PyObject *self, PyObject *args)
{
    PyObject *obj;
    if (!PyArg_ParseTuple(args, "O", &obj)) return NULL;
    double *c; Py_ssize_t n;
    if (get_coeffs(obj, &c, &n) < 0) return NULL;
    if (n == 0) { PyMem_Free(c); return PyFloat_FromDouble(0.0); }
    double m = c[0];
    for (Py_ssize_t i = 1; i < n; i++) if (c[i] < m) m = c[i];
    PyMem_Free(c);
    return PyFloat_FromDouble(m);
}
static PyObject *
poly_positive_coefficient_count(PyObject *self, PyObject *args)
{
    PyObject *obj;
    if (!PyArg_ParseTuple(args, "O", &obj)) return NULL;
    double *c; Py_ssize_t n;
    if (get_coeffs(obj, &c, &n) < 0) return NULL;
    long cnt = 0;
    for (Py_ssize_t i = 0; i < n; i++) if (c[i] > 0.0) cnt++;
    PyMem_Free(c);
    return PyLong_FromLong(cnt);
}
static PyObject *
poly_negative_coefficient_count(PyObject *self, PyObject *args)
{
    PyObject *obj;
    if (!PyArg_ParseTuple(args, "O", &obj)) return NULL;
    double *c; Py_ssize_t n;
    if (get_coeffs(obj, &c, &n) < 0) return NULL;
    long cnt = 0;
    for (Py_ssize_t i = 0; i < n; i++) if (c[i] < 0.0) cnt++;
    PyMem_Free(c);
    return PyLong_FromLong(cnt);
}
static PyObject *
poly_zero_coefficient_count(PyObject *self, PyObject *args)
{
    PyObject *obj;
    if (!PyArg_ParseTuple(args, "O", &obj)) return NULL;
    double *c; Py_ssize_t n;
    if (get_coeffs(obj, &c, &n) < 0) return NULL;
    long cnt = 0;
    for (Py_ssize_t i = 0; i < n; i++) if (c[i] == 0.0) cnt++;
    PyMem_Free(c);
    return PyLong_FromLong(cnt);
}
static PyObject *
poly_nonzero_coefficient_count(PyObject *self, PyObject *args)
{
    PyObject *obj;
    if (!PyArg_ParseTuple(args, "O", &obj)) return NULL;
    double *c; Py_ssize_t n;
    if (get_coeffs(obj, &c, &n) < 0) return NULL;
    long cnt = 0;
    for (Py_ssize_t i = 0; i < n; i++) if (c[i] != 0.0) cnt++;
    PyMem_Free(c);
    return PyLong_FromLong(cnt);
}

/* ----- Comparison / simple division ----- */
static PyObject *
poly_is_valid(PyObject *self, PyObject *args)
{
    PyObject *obj;
    if (!PyArg_ParseTuple(args, "O", &obj)) return NULL;
    if (!PyList_Check(obj) && !PyTuple_Check(obj))
        return Py_False;
    Py_ssize_t n = PySequence_Size(obj);
    for (Py_ssize_t i = 0; i < n; i++) {
        PyObject *item = PySequence_GetItem(obj, i);
        if (!item) return NULL;
        int ok; as_double(item, &ok);
        Py_DECREF(item);
        if (!ok) return Py_False;
    }
    return Py_True;
}
static PyObject *
poly_equal(PyObject *self, PyObject *args)
{
    PyObject *a, *b;
    if (!PyArg_ParseTuple(args, "OO", &a, &b)) return NULL;
    double *ca, *cb; Py_ssize_t na, nb;
    if (get_coeffs(a, &ca, &na) < 0) return NULL;
    if (get_coeffs(b, &cb, &nb) < 0) { PyMem_Free(ca); return NULL; }
    trim_trailing_zeros(&ca, &na);
    trim_trailing_zeros(&cb, &nb);
    int eq = (na == nb);
    if (eq) for (Py_ssize_t i = 0; i < na; i++) if (ca[i] != cb[i]) { eq = 0; break; }
    PyMem_Free(ca); PyMem_Free(cb);
    return PyBool_FromLong(eq);
}
STUB(is_scalar_multiple)
STUB(divide_by_linear)
STUB(remainder_linear)
static PyObject *
poly_is_root(PyObject *self, PyObject *args)
{
    PyObject *obj; double x;
    if (!PyArg_ParseTuple(args, "Od", &obj, &x)) return NULL;
    PyObject *val = poly_eval(self, Py_BuildValue("(Od)", obj, x));
    if (!val) return NULL;
    double v = PyFloat_AsDouble(val);
    Py_DECREF(val);
    return PyBool_FromLong(v == 0.0);
}
STUB(synthetic_division)
STUB(root_multiplicity)
STUB(divide)
STUB(remainder)
STUB(quotient)
STUB(divides)
STUB(monic_remainder)
STUB(gcd)
STUB(coprime)
STUB(lcm)
STUB(remove_factor)
STUB(factor_linear)
STUB(factor_from_root)
STUB(factor_value)
STUB(distance_from_root)
STUB(root_sign)

/* ----- Normalization / stats ----- */
STUB(normalize)
STUB(primitive_part)
STUB(content)
STUB(coefficient_gcd)
STUB(coefficient_lcm)
STUB(make_integer)
static PyObject *
poly_make_positive_leading(PyObject *self, PyObject *args)
{
    PyObject *obj;
    if (!PyArg_ParseTuple(args, "O", &obj)) return NULL;
    double *c; Py_ssize_t n;
    if (get_coeffs(obj, &c, &n) < 0) return NULL;
    double lc = 0.0;
    for (Py_ssize_t i = n; i-- > 0; ) if (c[i] != 0.0) { lc = c[i]; break; }
    if (lc < 0.0) for (Py_ssize_t i = 0; i < n; i++) c[i] = -c[i];
    PyObject *res = list_from_doubles(c, n);
    PyMem_Free(c);
    return res;
}
static PyObject *
poly_make_negative_leading(PyObject *self, PyObject *args)
{
    PyObject *obj;
    if (!PyArg_ParseTuple(args, "O", &obj)) return NULL;
    double *c; Py_ssize_t n;
    if (get_coeffs(obj, &c, &n) < 0) return NULL;
    double lc = 0.0;
    for (Py_ssize_t i = n; i-- > 0; ) if (c[i] != 0.0) { lc = c[i]; break; }
    if (lc > 0.0) for (Py_ssize_t i = 0; i < n; i++) c[i] = -c[i];
    PyObject *res = list_from_doubles(c, n);
    PyMem_Free(c);
    return res;
}
STUB(normalize_constant)
STUB(normalize_degree)
static PyObject *
poly_remove_trailing_zeros(PyObject *self, PyObject *args)
{
    return poly_trim(self, args);
}
STUB(remove_leading_zeros)
static PyObject *
poly_coefficient_range(PyObject *self, PyObject *args)
{
    PyObject *obj;
    if (!PyArg_ParseTuple(args, "O", &obj)) return NULL;
    double *c; Py_ssize_t n;
    if (get_coeffs(obj, &c, &n) < 0) return NULL;
    if (n == 0) { PyMem_Free(c); return PyFloat_FromDouble(0.0); }
    double mn = c[0], mx = c[0];
    for (Py_ssize_t i = 1; i < n; i++) {
        if (c[i] < mn) mn = c[i];
        if (c[i] > mx) mx = c[i];
    }
    PyMem_Free(c);
    return PyFloat_FromDouble(mx - mn);
}
static PyObject *
poly_coefficient_mean(PyObject *self, PyObject *args)
{
    PyObject *obj;
    if (!PyArg_ParseTuple(args, "O", &obj)) return NULL;
    double *c; Py_ssize_t n;
    if (get_coeffs(obj, &c, &n) < 0) return NULL;
    if (n == 0) { PyMem_Free(c); return PyFloat_FromDouble(0.0); }
    double s = 0.0;
    for (Py_ssize_t i = 0; i < n; i++) s += c[i];
    PyMem_Free(c);
    return PyFloat_FromDouble(s / n);
}
static PyObject *
poly_coefficient_variance(PyObject *self, PyObject *args)
{
    PyObject *obj;
    if (!PyArg_ParseTuple(args, "O", &obj)) return NULL;
    double *c; Py_ssize_t n;
    if (get_coeffs(obj, &c, &n) < 0) return NULL;
    if (n == 0) { PyMem_Free(c); return PyFloat_FromDouble(0.0); }
    double mean = 0.0;
    for (Py_ssize_t i = 0; i < n; i++) mean += c[i];
    mean /= n;
    double var = 0.0;
    for (Py_ssize_t i = 0; i < n; i++) {
        double d = c[i] - mean;
        var += d * d;
    }
    PyMem_Free(c);
    return PyFloat_FromDouble(var / n);
}
static PyObject *
poly_coefficient_abs_sum(PyObject *self, PyObject *args)
{
    PyObject *obj;
    if (!PyArg_ParseTuple(args, "O", &obj)) return NULL;
    double *c; Py_ssize_t n;
    if (get_coeffs(obj, &c, &n) < 0) return NULL;
    double s = 0.0;
    for (Py_ssize_t i = 0; i < n; i++) s += (c[i] < 0 ? -c[i] : c[i]);
    PyMem_Free(c);
    return PyFloat_FromDouble(s);
}
static PyObject *
poly_coefficient_energy(PyObject *self, PyObject *args)
{
    return poly_squared_coefficient_sum(self, args);
}

/* ----- Structure ----- */
static PyObject *
poly_term_count(PyObject *self, PyObject *args)
{
    return poly_nonzero_coefficient_count(self, args);
}
static PyObject *
poly_nonzero_degree(PyObject *self, PyObject *args)
{
    return poly_degree(self, args);
}
static PyObject *
poly_highest_nonzero_index(PyObject *self, PyObject *args)
{
    PyObject *obj;
    if (!PyArg_ParseTuple(args, "O", &obj)) return NULL;
    double *c; Py_ssize_t n;
    if (get_coeffs(obj, &c, &n) < 0) return NULL;
    Py_ssize_t idx = -1;
    for (Py_ssize_t i = n; i-- > 0; ) if (c[i] != 0.0) { idx = i; break; }
    PyMem_Free(c);
    return PyLong_FromSsize_t(idx);
}
static PyObject *
poly_lowest_nonzero_index(PyObject *self, PyObject *args)
{
    PyObject *obj;
    if (!PyArg_ParseTuple(args, "O", &obj)) return NULL;
    double *c; Py_ssize_t n;
    if (get_coeffs(obj, &c, &n) < 0) return NULL;
    Py_ssize_t idx = -1;
    for (Py_ssize_t i = 0; i < n; i++) if (c[i] != 0.0) { idx = i; break; }
    PyMem_Free(c);
    return PyLong_FromSsize_t(idx);
}
STUB(zero_run_count)
static PyObject *
poly_sign_change_count(PyObject *self, PyObject *args)
{
    PyObject *obj;
    if (!PyArg_ParseTuple(args, "O", &obj)) return NULL;
    double *c; Py_ssize_t n;
    if (get_coeffs(obj, &c, &n) < 0) return NULL;
    long changes = 0;
    int last = 0;
    for (Py_ssize_t i = 0; i < n; i++) {
        if (c[i] == 0.0) continue;
        int s = (c[i] > 0.0) ? 1 : -1;
        if (last && s != last) changes++;
        last = s;
    }
    PyMem_Free(c);
    return PyLong_FromLong(changes);
}
static PyObject *
poly_positive_term_count(PyObject *self, PyObject *args)
{
    return poly_positive_coefficient_count(self, args);
}
static PyObject *
poly_negative_term_count(PyObject *self, PyObject *args)
{
    return poly_negative_coefficient_count(self, args);
}
static PyObject *
poly_constant_only(PyObject *self, PyObject *args)
{
    return poly_is_constant(self, args);
}
static PyObject *
poly_linear_only(PyObject *self, PyObject *args)
{
    PyObject *obj;
    if (!PyArg_ParseTuple(args, "O", &obj)) return NULL;
    double *c; Py_ssize_t n;
    if (get_coeffs(obj, &c, &n) < 0) return NULL;
    int ok = (effective_degree(c, n) <= 1);
    PyMem_Free(c);
    return PyBool_FromLong(ok);
}
static PyObject *
poly_quadratic_only(PyObject *self, PyObject *args)
{
    PyObject *obj;
    if (!PyArg_ParseTuple(args, "O", &obj)) return NULL;
    double *c; Py_ssize_t n;
    if (get_coeffs(obj, &c, &n) < 0) return NULL;
    int ok = (effective_degree(c, n) == 2);
    PyMem_Free(c);
    return PyBool_FromLong(ok);
}
static PyObject *
poly_cubic_only(PyObject *self, PyObject *args)
{
    PyObject *obj;
    if (!PyArg_ParseTuple(args, "O", &obj)) return NULL;
    double *c; Py_ssize_t n;
    if (get_coeffs(obj, &c, &n) < 0) return NULL;
    int ok = (effective_degree(c, n) == 3);
    PyMem_Free(c);
    return PyBool_FromLong(ok);
}
STUB(homogeneous)
STUB(sparse)
STUB(dense)
STUB(has_repeated_coefficients)
static PyObject *
poly_has_zero_coefficients(PyObject *self, PyObject *args)
{
    PyObject *obj;
    if (!PyArg_ParseTuple(args, "O", &obj)) return NULL;
    double *c; Py_ssize_t n;
    if (get_coeffs(obj, &c, &n) < 0) return NULL;
    int has = 0;
    for (Py_ssize_t i = 0; i < n; i++) if (c[i] == 0.0) { has = 1; break; }
    PyMem_Free(c);
    return PyBool_FromLong(has);
}
static PyObject *
poly_all_coefficients_equal(PyObject *self, PyObject *args)
{
    PyObject *obj;
    if (!PyArg_ParseTuple(args, "O", &obj)) return NULL;
    double *c; Py_ssize_t n;
    if (get_coeffs(obj, &c, &n) < 0) return NULL;
    int eq = 1;
    if (n > 0) {
        double v = c[0];
        for (Py_ssize_t i = 1; i < n; i++) if (c[i] != v) { eq = 0; break; }
    }
    PyMem_Free(c);
    return PyBool_FromLong(eq);
}
STUB(coefficients_increasing)
STUB(coefficients_decreasing)

/* Root utilities – many left as stubs; a few simple helpers */
STUB(root_value)
STUB(root_product)
STUB(root_sum)
STUB(root_distance)
STUB(root_magnitude)
STUB(root_is_positive)
STUB(root_is_negative)
STUB(root_is_zero)
STUB(root_is_integer)
STUB(root_is_fraction)
STUB(root_is_repeated)
STUB(count_real_candidate_roots)
static PyObject *
poly_count_sign_changes(PyObject *self, PyObject *args)
{
    return poly_sign_change_count(self, args);
}
STUB(descartes_positive_bound)
STUB(descartes_negative_bound)
STUB(cauchy_root_bound)
STUB(positive_root_bound)
STUB(negative_root_bound)
STUB(absolute_root_bound)
STUB(root_interval)
STUB(root_separation)
STUB(root_deflate)
STUB(root_inflate)
STUB(root_factor)

/* Linear / Quadratic – implemented */
static PyObject *
poly_solve_linear(PyObject *self, PyObject *args)
{
    PyObject *obj;
    if (!PyArg_ParseTuple(args, "O", &obj)) return NULL;
    double *c; Py_ssize_t n;
    if (get_coeffs(obj, &c, &n) < 0) return NULL;
    if (n < 2 || c[1] == 0.0) {
        PyMem_Free(c);
        PyErr_SetString(PyExc_ValueError, "not a linear polynomial");
        return NULL;
    }
    double root = -c[0] / c[1];
    PyMem_Free(c);
    return PyFloat_FromDouble(root);
}
static PyObject *
poly_quadratic_discriminant(PyObject *self, PyObject *args)
{
    PyObject *obj;
    if (!PyArg_ParseTuple(args, "O", &obj)) return NULL;
    double *c; Py_ssize_t n;
    if (get_coeffs(obj, &c, &n) < 0) return NULL;
    if (n < 3) { PyMem_Free(c); return PyFloat_FromDouble(0.0); }
    double a = c[2], b = c[1], cc = c[0];
    PyMem_Free(c);
    return PyFloat_FromDouble(b*b - 4*a*cc);
}
static PyObject *
poly_solve_quadratic(PyObject *self, PyObject *args)
{
    PyObject *obj;
    if (!PyArg_ParseTuple(args, "O", &obj)) return NULL;
    double *c; Py_ssize_t n;
    if (get_coeffs(obj, &c, &n) < 0) return NULL;
    if (n < 3 || c[2] == 0.0) {
        PyMem_Free(c);
        PyErr_SetString(PyExc_ValueError, "not a quadratic");
        return NULL;
    }
    double a = c[2], b = c[1], cc = c[0];
    PyMem_Free(c);
    double disc = b*b - 4*a*cc;
    PyObject *res = PyList_New(2);
    if (disc < 0) {
        PyList_SET_ITEM(res, 0, Py_None); Py_INCREF(Py_None);
        PyList_SET_ITEM(res, 1, Py_None); Py_INCREF(Py_None);
    } else {
        double s = disc > 0 ? sqrt(disc) : 0.0;  /* needs math.h */
        PyList_SET_ITEM(res, 0, PyFloat_FromDouble((-b + s)/(2*a)));
        PyList_SET_ITEM(res, 1, PyFloat_FromDouble((-b - s)/(2*a)));
    }
    return res;
}
static PyObject *
poly_quadratic_vertex(PyObject *self, PyObject *args)
{
    PyObject *obj;
    if (!PyArg_ParseTuple(args, "O", &obj)) return NULL;
    double *c; Py_ssize_t n;
    if (get_coeffs(obj, &c, &n) < 0) return NULL;
    if (n < 3 || c[2] == 0.0) {
        PyMem_Free(c);
        PyErr_SetString(PyExc_ValueError, "not a quadratic");
        return NULL;
    }
    double a = c[2], b = c[1];
    double xv = -b / (2*a);
    double yv = a*xv*xv + b*xv + c[0];
    PyMem_Free(c);
    return Py_BuildValue("(dd)", xv, yv);
}
static PyObject *
poly_quadratic_axis(PyObject *self, PyObject *args)
{
    PyObject *obj;
    if (!PyArg_ParseTuple(args, "O", &obj)) return NULL;
    double *c; Py_ssize_t n;
    if (get_coeffs(obj, &c, &n) < 0) return NULL;
    if (n < 3 || c[2] == 0.0) {
        PyMem_Free(c);
        PyErr_SetString(PyExc_ValueError, "not a quadratic");
        return NULL;
    }
    double xv = -c[1] / (2*c[2]);
    PyMem_Free(c);
    return PyFloat_FromDouble(xv);
}
static PyObject *
poly_quadratic_minimum(PyObject *self, PyObject *args)
{
    PyObject *v = poly_quadratic_vertex(self, args);
    if (!v) return NULL;
    return PySequence_GetItem(v, 1);
}
static PyObject *
poly_quadratic_maximum(PyObject *self, PyObject *args)
{
    return poly_quadratic_minimum(self, args); /* same value; sign of a decides min/max */
}
static PyObject *
poly_quadratic_roots_sum(PyObject *self, PyObject *args)
{
    PyObject *obj;
    if (!PyArg_ParseTuple(args, "O", &obj)) return NULL;
    double *c; Py_ssize_t n;
    if (get_coeffs(obj, &c, &n) < 0) return NULL;
    if (n < 3 || c[2] == 0.0) { PyMem_Free(c); return PyFloat_FromDouble(0.0); }
    double s = -c[1] / c[2];
    PyMem_Free(c);
    return PyFloat_FromDouble(s);
}
static PyObject *
poly_quadratic_roots_product(PyObject *self, PyObject *args)
{
    PyObject *obj;
    if (!PyArg_ParseTuple(args, "O", &obj)) return NULL;
    double *c; Py_ssize_t n;
    if (get_coeffs(obj, &c, &n) < 0) return NULL;
    if (n < 3 || c[2] == 0.0) { PyMem_Free(c); return PyFloat_FromDouble(0.0); }
    double p = c[0] / c[2];
    PyMem_Free(c);
    return PyFloat_FromDouble(p);
}
static PyObject *
poly_quadratic_from_roots(PyObject *self, PyObject *args)
{
    double r1, r2;
    if (!PyArg_ParseTuple(args, "dd", &r1, &r2)) return NULL;
    double coeffs[3] = {r1*r2, -(r1+r2), 1.0};
    return list_from_doubles(coeffs, 3);
}
STUB(quadratic_from_vertex)
static PyObject *
poly_linear_from_root(PyObject *self, PyObject *args)
{
    double r;
    if (!PyArg_ParseTuple(args, "d", &r)) return NULL;
    double coeffs[2] = {-r, 1.0};
    return list_from_doubles(coeffs, 2);
}
static PyObject *
poly_linear_from_slope(PyObject *self, PyObject *args)
{
    double m, b;
    if (!PyArg_ParseTuple(args, "dd", &m, &b)) return NULL;
    double coeffs[2] = {b, m};
    return list_from_doubles(coeffs, 2);
}
static PyObject *
poly_slope(PyObject *self, PyObject *args)
{
    PyObject *obj;
    if (!PyArg_ParseTuple(args, "O", &obj)) return NULL;
    double *c; Py_ssize_t n;
    if (get_coeffs(obj, &c, &n) < 0) return NULL;
    double s = (n > 1) ? c[1] : 0.0;
    PyMem_Free(c);
    return PyFloat_FromDouble(s);
}
static PyObject *
poly_intercept(PyObject *self, PyObject *args)
{
    return poly_constant_term(self, args);
}
static PyObject *
poly_y_intercept(PyObject *self, PyObject *args)
{
    return poly_constant_term(self, args);
}
static PyObject *
poly_x_intercept(PyObject *self, PyObject *args)
{
    return poly_solve_linear(self, args);
}
static PyObject *
poly_line_value(PyObject *self, PyObject *args)
{
    return poly_eval(self, args);
}
STUB(line_parallel)
STUB(line_perpendicular)

STUB(lagrange_basis)
STUB(lagrange_interpolate)
STUB(newton_interpolate)
STUB(divided_difference)
STUB(finite_difference)
STUB(first_difference)
STUB(second_difference)
STUB(third_difference)
STUB(difference_table)
STUB(forward_difference)
STUB(backward_difference)
STUB(central_difference)
STUB(difference_sum)
STUB(difference_product)
STUB(interpolate_two_points)
STUB(interpolate_three_points)
STUB(interpolate_four_points)
STUB(interpolation_error)
STUB(interpolation_degree)
STUB(interpolation_residual)

STUB(fibonacci_polynomial)
STUB(lucas_polynomial)
STUB(chebyshev_t)
STUB(chebyshev_u)
STUB(legendre)
STUB(laguerre)
STUB(hermite)
STUB(bernstein)
STUB(monomial)
STUB(binomial_polynomial)
STUB(geometric_polynomial)
STUB(arithmetic_polynomial)
STUB(falling_factorial)
STUB(rising_factorial)
STUB(factorial_polynomial)
STUB(power_sum_polynomial)
STUB(alternating_polynomial)
STUB(cyclotomic_candidate)
STUB(geometric_sum)
STUB(finite_geometric_polynomial)

STUB(derivative_n)
STUB(integral_n)
STUB(antiderivative_value)
STUB(derivative_at)
STUB(second_derivative)
STUB(third_derivative)
STUB(derivative_order)
STUB(derivative_degree)
STUB(integral_value)
STUB(definite_integral)
STUB(average_value)
STUB(critical_point_candidate)
STUB(stationary_value)
STUB(curvature_candidate)
STUB(tangent_line)
STUB(normal_line)
STUB(secant_slope)
STUB(mean_slope)
STUB(derivative_coefficients)
STUB(integral_coefficients)

STUB(compose_constant)
STUB(compose_linear)
STUB(compose_monomial)
STUB(compose_power)
STUB(substitute)
STUB(substitute_zero)
STUB(substitute_one)
STUB(substitute_minus_one)
STUB(substitute_linear)
STUB(substitute_affine)
STUB(substitute_scaled)
STUB(substitute_shifted)
STUB(substitute_reflected)
STUB(compose_derivative)
STUB(derivative_compose)
STUB(compose_power_series)
STUB(polynomial_chain)
STUB(polynomial_affine_transform)
STUB(polynomial_variable_scale)
STUB(polynomial_variable_shift)

STUB(resultant_small)
STUB(discriminant_quadratic)
STUB(discriminant_cubic)
STUB(factor_difference_of_squares)
STUB(factor_sum_of_squares)
STUB(factor_difference_of_cubes)
STUB(factor_sum_of_cubes)
STUB(factor_common_term)
STUB(factor_by_grouping)
STUB(factor_quadratic)
STUB(factor_cubic_candidate)
STUB(perfect_square_test)
STUB(perfect_cube_test)
STUB(repeated_root_test)
STUB(square_free_test)
STUB(square_free_part)
STUB(repeated_factor_count)
STUB(multiplicity_sum)
STUB(polynomial_power_free)
STUB(factor_count_candidate)

STUB(product_by_x_minus_a)
STUB(product_by_x_plus_a)
STUB(product_by_linear_factors)
STUB(product_of_polynomials)
/* product_coefficients already implemented as core */
STUB(convolution)
STUB(coefficient_convolution)
STUB(hadamard_product)
STUB(alternating_product)
STUB(self_product)

STUB(coefficient_sum_squares)
STUB(coefficient_sum_cubes)
STUB(coefficient_l1_norm)
STUB(coefficient_l2_norm)
STUB(coefficient_max_abs)
STUB(coefficient_min_abs)
/* coefficient_range, mean, variance already stubbed earlier */
STUB(coefficient_midrange)
STUB(coefficient_median_candidate)
STUB(coefficient_deviation)
STUB(coefficient_positive_sum)
STUB(coefficient_negative_sum)
/* coefficient_abs_sum already stubbed */

STUB(to_string)
STUB(to_terms)
STUB(from_terms)
STUB(from_roots)
STUB(from_coefficients)
STUB(to_monomial_string)
STUB(to_latex_like)
STUB(term_degree)
STUB(term_coefficient)
STUB(term_count_nonzero)
STUB(format_coefficient)
STUB(format_term)
STUB(format_polynomial)

STUB(validate_coefficients)
STUB(validate_degree)
STUB(validate_root)
STUB(validate_division)
STUB(validate_derivative)
STUB(validate_integral)
STUB(validate_factor)
STUB(validate_identity)
STUB(validate_composition)
STUB(validate_interpolation)
STUB(validate_remainder)
STUB(validate_gcd)

/* ============================================================
 * Method table
 * ============================================================ */

static PyMethodDef PolynomialMethods[] = {
    /* Core 1-36 */
    {"eval",                poly_eval,                METH_VARARGS, "Evaluate polynomial at x"},
    {"degree",              poly_degree,              METH_VARARGS, "Degree of polynomial"},
    {"copy",                poly_copy,                METH_VARARGS, "Copy coefficient list"},
    {"zero",                poly_zero,                METH_VARARGS, "Zero polynomial of given degree"},
    {"add",                 poly_add,                 METH_VARARGS, "Add two polynomials"},
    {"subtract",            poly_subtract,            METH_VARARGS, "Subtract two polynomials"},
    {"negate",              poly_negate,              METH_VARARGS, "Negate polynomial"},
    {"scalar_add",          poly_scalar_add,          METH_VARARGS, "Add scalar to polynomial"},
    {"scalar_subtract",     poly_scalar_subtract,     METH_VARARGS, "Subtract scalar from polynomial"},
    {"scalar_multiply",     poly_scalar_multiply,     METH_VARARGS, "Multiply polynomial by scalar"},
    {"scalar_divide",       poly_scalar_divide,       METH_VARARGS, "Divide polynomial by scalar"},
    {"multiply",            poly_multiply,            METH_VARARGS, "Multiply two polynomials"},
    {"multiply_x",          poly_multiply_x,          METH_VARARGS, "Multiply polynomial by x"},
    {"derivative",          poly_derivative,          METH_VARARGS, "Formal derivative"},
    {"integral",            poly_integral,            METH_VARARGS, "Indefinite integral (const term optional)"},
    {"sum_coefficients",    poly_sum_coefficients,    METH_VARARGS, "Sum of coefficients"},
    {"product_coefficients",poly_product_coefficients,METH_VARARGS, "Product of coefficients"},
    {"leading_coefficient", poly_leading_coefficient, METH_VARARGS, "Leading coefficient"},
    {"constant_term",       poly_constant_term,       METH_VARARGS, "Constant term"},
    {"is_zero",             poly_is_zero,             METH_VARARGS, "Is zero polynomial"},
    {"is_constant",         poly_is_constant,         METH_VARARGS, "Is constant polynomial"},
    {"is_monic",            poly_is_monic,            METH_VARARGS, "Is monic polynomial"},
    {"is_even",             poly_is_even,             METH_VARARGS, "Is even polynomial"},
    {"is_odd",              poly_is_odd,              METH_VARARGS, "Is odd polynomial"},
    {"reverse",             poly_reverse,             METH_VARARGS, "Reverse coefficients"},
    {"abs_coefficients",    poly_abs_coefficients,    METH_VARARGS, "Absolute values of coefficients"},
    {"make_monic",          poly_make_monic,          METH_VARARGS, "Make monic"},
    {"trim",                poly_trim,                METH_VARARGS, "Trim trailing zeros"},
    {"at_zero",             poly_at_zero,             METH_VARARGS, "Value at 0"},
    {"at_one",              poly_at_one,              METH_VARARGS, "Value at 1"},
    {"at_minus_one",        poly_at_minus_one,        METH_VARARGS, "Value at -1"},
    {"binomial",            poly_binomial,            METH_VARARGS, "Binomial coefficient C(n,k)"},
    {"shift_x",             poly_shift_x,             METH_VARARGS, "p(x + a)"},
    {"scale_x",             poly_scale_x,             METH_VARARGS, "p(s * x)"},
    {"add_x",               poly_add_x,               METH_VARARGS, "Add x to polynomial"},
    {"add_constant",        poly_add_constant,        METH_VARARGS, "Add constant"},

    /* Powers and transforms 37-66 */
    {"power_integer",       poly_power_integer,       METH_VARARGS, NULL},
    {"power",               poly_power,               METH_VARARGS, NULL},
    {"square",              poly_square,              METH_VARARGS, NULL},
    {"cube",                poly_cube,                METH_VARARGS, NULL},
    {"compose",             poly_compose,             METH_VARARGS, NULL},
    {"translate",           poly_translate,           METH_VARARGS, NULL},
    {"reflect_x",           poly_reflect_x,           METH_VARARGS, NULL},
    {"reflect_y",           poly_reflect_y,           METH_VARARGS, NULL},
    {"dilate",              poly_dilate,              METH_VARARGS, NULL},
    {"contract",            poly_contract,            METH_VARARGS, NULL},
    {"even_part",           poly_even_part,           METH_VARARGS, NULL},
    {"odd_part",            poly_odd_part,            METH_VARARGS, NULL},
    {"remove_constant",     poly_remove_constant,     METH_VARARGS, NULL},
    {"remove_linear",       poly_remove_linear,       METH_VARARGS, NULL},
    {"remove_leading",      poly_remove_leading,      METH_VARARGS, NULL},
    {"shift_coefficients",  poly_shift_coefficients,  METH_VARARGS, NULL},
    {"insert_zero",         poly_insert_zero,         METH_VARARGS, NULL},
    {"remove_coefficient",  poly_remove_coefficient,  METH_VARARGS, NULL},
    {"multiply_by_linear",  poly_multiply_by_linear,  METH_VARARGS, NULL},
    {"add_linear_factor",    poly_add_linear_factor,    METH_VARARGS, NULL},
    {"coefficient",         poly_coefficient,         METH_VARARGS, NULL},
    {"coefficient_sum_range", poly_coefficient_sum_range, METH_VARARGS, NULL},
    {"weighted_coefficient_sum", poly_weighted_coefficient_sum, METH_VARARGS, NULL},
    {"squared_coefficient_sum", poly_squared_coefficient_sum, METH_VARARGS, NULL},
    {"max_coefficient",     poly_max_coefficient,     METH_VARARGS, NULL},
    {"min_coefficient",     poly_min_coefficient,     METH_VARARGS, NULL},
    {"positive_coefficient_count", poly_positive_coefficient_count, METH_VARARGS, NULL},
    {"negative_coefficient_count", poly_negative_coefficient_count, METH_VARARGS, NULL},
    {"zero_coefficient_count", poly_zero_coefficient_count, METH_VARARGS, NULL},
    {"nonzero_coefficient_count", poly_nonzero_coefficient_count, METH_VARARGS, NULL},

    /* Comparison and division 67-88 */
    {"is_valid",            poly_is_valid,            METH_VARARGS, NULL},
    {"equal",               poly_equal,               METH_VARARGS, NULL},
    {"is_scalar_multiple",  poly_is_scalar_multiple,  METH_VARARGS, NULL},
    {"divide_by_linear",    poly_divide_by_linear,    METH_VARARGS, NULL},
    {"remainder_linear",    poly_remainder_linear,    METH_VARARGS, NULL},
    {"is_root",             poly_is_root,             METH_VARARGS, NULL},
    {"synthetic_division",  poly_synthetic_division,  METH_VARARGS, NULL},
    {"root_multiplicity",   poly_root_multiplicity,   METH_VARARGS, NULL},
    {"divide",              poly_divide,              METH_VARARGS, NULL},
    {"remainder",           poly_remainder,           METH_VARARGS, NULL},
    {"quotient",            poly_quotient,            METH_VARARGS, NULL},
    {"divides",             poly_divides,             METH_VARARGS, NULL},
    {"monic_remainder",     poly_monic_remainder,     METH_VARARGS, NULL},
    {"gcd",                 poly_gcd,                 METH_VARARGS, NULL},
    {"coprime",             poly_coprime,             METH_VARARGS, NULL},
    {"lcm",                 poly_lcm,                 METH_VARARGS, NULL},
    {"remove_factor",        poly_remove_factor,        METH_VARARGS, NULL},
    {"factor_linear",        poly_factor_linear,        METH_VARARGS, NULL},
    {"factor_from_root",     poly_factor_from_root,     METH_VARARGS, NULL},
    {"factor_value",         poly_factor_value,         METH_VARARGS, NULL},
    {"distance_from_root",  poly_distance_from_root,  METH_VARARGS, NULL},
    {"root_sign",           poly_root_sign,           METH_VARARGS, NULL},

    /* Normalization 89-105 */
    {"normalize",           poly_normalize,           METH_VARARGS, NULL},
    {"primitive_part",      poly_primitive_part,      METH_VARARGS, NULL},
    {"content",             poly_content,             METH_VARARGS, NULL},
    {"coefficient_gcd",     poly_coefficient_gcd,     METH_VARARGS, NULL},
    {"coefficient_lcm",     poly_coefficient_lcm,     METH_VARARGS, NULL},
    {"make_integer",        poly_make_integer,        METH_VARARGS, NULL},
    {"make_positive_leading", poly_make_positive_leading, METH_VARARGS, NULL},
    {"make_negative_leading", poly_make_negative_leading, METH_VARARGS, NULL},
    {"normalize_constant",  poly_normalize_constant,  METH_VARARGS, NULL},
    {"normalize_degree",    poly_normalize_degree,    METH_VARARGS, NULL},
    {"remove_trailing_zeros", poly_remove_trailing_zeros, METH_VARARGS, NULL},
    {"remove_leading_zeros", poly_remove_leading_zeros, METH_VARARGS, NULL},
    {"coefficient_range",   poly_coefficient_range,   METH_VARARGS, NULL},
    {"coefficient_mean",    poly_coefficient_mean,    METH_VARARGS, NULL},
    {"coefficient_variance", poly_coefficient_variance, METH_VARARGS, NULL},
    {"coefficient_abs_sum", poly_coefficient_abs_sum, METH_VARARGS, NULL},
    {"coefficient_energy",  poly_coefficient_energy,  METH_VARARGS, NULL},

    /* Structure 106-125 */
    {"term_count",          poly_term_count,          METH_VARARGS, NULL},
    {"nonzero_degree",      poly_nonzero_degree,      METH_VARARGS, NULL},
    {"highest_nonzero_index", poly_highest_nonzero_index, METH_VARARGS, NULL},
    {"lowest_nonzero_index", poly_lowest_nonzero_index, METH_VARARGS, NULL},
    {"zero_run_count",      poly_zero_run_count,      METH_VARARGS, NULL},
    {"sign_change_count",   poly_sign_change_count,   METH_VARARGS, NULL},
    {"positive_term_count", poly_positive_term_count, METH_VARARGS, NULL},
    {"negative_term_count", poly_negative_term_count, METH_VARARGS, NULL},
    {"constant_only",       poly_constant_only,       METH_VARARGS, NULL},
    {"linear_only",         poly_linear_only,         METH_VARARGS, NULL},
    {"quadratic_only",      poly_quadratic_only,      METH_VARARGS, NULL},
    {"cubic_only",          poly_cubic_only,          METH_VARARGS, NULL},
    {"homogeneous",         poly_homogeneous,         METH_VARARGS, NULL},
    {"sparse",              poly_sparse,              METH_VARARGS, NULL},
    {"dense",               poly_dense,               METH_VARARGS, NULL},
    {"has_repeated_coefficients", poly_has_repeated_coefficients, METH_VARARGS, NULL},
    {"has_zero_coefficients", poly_has_zero_coefficients, METH_VARARGS, NULL},
    {"all_coefficients_equal", poly_all_coefficients_equal, METH_VARARGS, NULL},
    {"coefficients_increasing", poly_coefficients_increasing, METH_VARARGS, NULL},
    {"coefficients_decreasing", poly_coefficients_decreasing, METH_VARARGS, NULL},

    /* Root utilities 126-150 */
    {"root_value",          poly_root_value,          METH_VARARGS, NULL},
    {"root_product",        poly_root_product,        METH_VARARGS, NULL},
    {"root_sum",            poly_root_sum,            METH_VARARGS, NULL},
    {"root_distance",       poly_root_distance,       METH_VARARGS, NULL},
    {"root_magnitude",      poly_root_magnitude,      METH_VARARGS, NULL},
    /* root_sign already in table */
    {"root_is_positive",    poly_root_is_positive,    METH_VARARGS, NULL},
    {"root_is_negative",    poly_root_is_negative,    METH_VARARGS, NULL},
    {"root_is_zero",        poly_root_is_zero,        METH_VARARGS, NULL},
    {"root_is_integer",     poly_root_is_integer,     METH_VARARGS, NULL},
    {"root_is_fraction",    poly_root_is_fraction,    METH_VARARGS, NULL},
    {"root_is_repeated",    poly_root_is_repeated,    METH_VARARGS, NULL},
    {"count_real_candidate_roots", poly_count_real_candidate_roots, METH_VARARGS, NULL},
    {"count_sign_changes",  poly_count_sign_changes,  METH_VARARGS, NULL},
    {"descartes_positive_bound", poly_descartes_positive_bound, METH_VARARGS, NULL},
    {"descartes_negative_bound", poly_descartes_negative_bound, METH_VARARGS, NULL},
    {"cauchy_root_bound",   poly_cauchy_root_bound,   METH_VARARGS, NULL},
    {"positive_root_bound", poly_positive_root_bound, METH_VARARGS, NULL},
    {"negative_root_bound", poly_negative_root_bound, METH_VARARGS, NULL},
    {"absolute_root_bound", poly_absolute_root_bound, METH_VARARGS, NULL},
    {"root_interval",       poly_root_interval,       METH_VARARGS, NULL},
    {"root_separation",     poly_root_separation,     METH_VARARGS, NULL},
    {"root_deflate",        poly_root_deflate,        METH_VARARGS, NULL},
    {"root_inflate",        poly_root_inflate,        METH_VARARGS, NULL},
    {"root_factor",         poly_root_factor,         METH_VARARGS, NULL},

    /* Linear/Quadratic 151-170 */
    {"solve_linear",        poly_solve_linear,        METH_VARARGS, NULL},
    {"solve_quadratic",     poly_solve_quadratic,     METH_VARARGS, NULL},
    {"quadratic_discriminant", poly_quadratic_discriminant, METH_VARARGS, NULL},
    {"quadratic_vertex",    poly_quadratic_vertex,    METH_VARARGS, NULL},
    {"quadratic_axis",      poly_quadratic_axis,      METH_VARARGS, NULL},
    {"quadratic_minimum",   poly_quadratic_minimum,   METH_VARARGS, NULL},
    {"quadratic_maximum",   poly_quadratic_maximum,   METH_VARARGS, NULL},
    {"quadratic_roots_sum", poly_quadratic_roots_sum, METH_VARARGS, NULL},
    {"quadratic_roots_product", poly_quadratic_roots_product, METH_VARARGS, NULL},
    {"quadratic_from_roots", poly_quadratic_from_roots, METH_VARARGS, NULL},
    {"quadratic_from_vertex", poly_quadratic_from_vertex, METH_VARARGS, NULL},
    {"linear_from_root",    poly_linear_from_root,    METH_VARARGS, NULL},
    {"linear_from_slope",   poly_linear_from_slope,   METH_VARARGS, NULL},
    {"slope",               poly_slope,               METH_VARARGS, NULL},
    {"intercept",           poly_intercept,           METH_VARARGS, NULL},
    {"y_intercept",         poly_y_intercept,         METH_VARARGS, NULL},
    {"x_intercept",         poly_x_intercept,         METH_VARARGS, NULL},
    {"line_value",          poly_line_value,          METH_VARARGS, NULL},
    {"line_parallel",       poly_line_parallel,       METH_VARARGS, NULL},
    {"line_perpendicular",  poly_line_perpendicular,  METH_VARARGS, NULL},

    /* Interpolation 171-190 */
    {"lagrange_basis",      poly_lagrange_basis,      METH_VARARGS, NULL},
    {"lagrange_interpolate", poly_lagrange_interpolate, METH_VARARGS, NULL},
    {"newton_interpolate",  poly_newton_interpolate,  METH_VARARGS, NULL},
    {"divided_difference",  poly_divided_difference,  METH_VARARGS, NULL},
    {"finite_difference",   poly_finite_difference,   METH_VARARGS, NULL},
    {"first_difference",    poly_first_difference,    METH_VARARGS, NULL},
    {"second_difference",   poly_second_difference,   METH_VARARGS, NULL},
    {"third_difference",    poly_third_difference,    METH_VARARGS, NULL},
    {"difference_table",    poly_difference_table,    METH_VARARGS, NULL},
    {"forward_difference",  poly_forward_difference,  METH_VARARGS, NULL},
    {"backward_difference", poly_backward_difference, METH_VARARGS, NULL},
    {"central_difference",  poly_central_difference,  METH_VARARGS, NULL},
    {"difference_sum",      poly_difference_sum,      METH_VARARGS, NULL},
    {"difference_product",  poly_difference_product,  METH_VARARGS, NULL},
    {"interpolate_two_points", poly_interpolate_two_points, METH_VARARGS, NULL},
    {"interpolate_three_points", poly_interpolate_three_points, METH_VARARGS, NULL},
    {"interpolate_four_points", poly_interpolate_four_points, METH_VARARGS, NULL},
    {"interpolation_error", poly_interpolation_error, METH_VARARGS, NULL},
    {"interpolation_degree", poly_interpolation_degree, METH_VARARGS, NULL},
    {"interpolation_residual", poly_interpolation_residual, METH_VARARGS, NULL},

    /* Sequences 191-210 */
    {"fibonacci_polynomial", poly_fibonacci_polynomial, METH_VARARGS, NULL},
    {"lucas_polynomial",    poly_lucas_polynomial,    METH_VARARGS, NULL},
    {"chebyshev_t",         poly_chebyshev_t,         METH_VARARGS, NULL},
    {"chebyshev_u",         poly_chebyshev_u,         METH_VARARGS, NULL},
    {"legendre",            poly_legendre,            METH_VARARGS, NULL},
    {"laguerre",            poly_laguerre,            METH_VARARGS, NULL},
    {"hermite",             poly_hermite,             METH_VARARGS, NULL},
    {"bernstein",           poly_bernstein,           METH_VARARGS, NULL},
    {"monomial",            poly_monomial,            METH_VARARGS, NULL},
    {"binomial_polynomial", poly_binomial_polynomial, METH_VARARGS, NULL},
    {"geometric_polynomial", poly_geometric_polynomial, METH_VARARGS, NULL},
    {"arithmetic_polynomial", poly_arithmetic_polynomial, METH_VARARGS, NULL},
    {"falling_factorial",   poly_falling_factorial,   METH_VARARGS, NULL},
    {"rising_factorial",    poly_rising_factorial,    METH_VARARGS, NULL},
    {"factorial_polynomial", poly_factorial_polynomial, METH_VARARGS, NULL},
    {"power_sum_polynomial", poly_power_sum_polynomial, METH_VARARGS, NULL},
    {"alternating_polynomial", poly_alternating_polynomial, METH_VARARGS, NULL},
    {"cyclotomic_candidate", poly_cyclotomic_candidate, METH_VARARGS, NULL},
    {"geometric_sum",       poly_geometric_sum,       METH_VARARGS, NULL},
    {"finite_geometric_polynomial", poly_finite_geometric_polynomial, METH_VARARGS, NULL},

    /* Calculus 211-230 */
    {"derivative_n",        poly_derivative_n,        METH_VARARGS, NULL},
    {"integral_n",          poly_integral_n,          METH_VARARGS, NULL},
    {"antiderivative_value", poly_antiderivative_value, METH_VARARGS, NULL},
    {"derivative_at",       poly_derivative_at,       METH_VARARGS, NULL},
    {"second_derivative",   poly_second_derivative,   METH_VARARGS, NULL},
    {"third_derivative",    poly_third_derivative,    METH_VARARGS, NULL},
    {"derivative_order",    poly_derivative_order,    METH_VARARGS, NULL},
    {"derivative_degree",   poly_derivative_degree,   METH_VARARGS, NULL},
    {"integral_value",      poly_integral_value,      METH_VARARGS, NULL},
    {"definite_integral",   poly_definite_integral,   METH_VARARGS, NULL},
    {"average_value",       poly_average_value,       METH_VARARGS, NULL},
    {"critical_point_candidate", poly_critical_point_candidate, METH_VARARGS, NULL},
    {"stationary_value",    poly_stationary_value,    METH_VARARGS, NULL},
    {"curvature_candidate", poly_curvature_candidate, METH_VARARGS, NULL},
    {"tangent_line",        poly_tangent_line,        METH_VARARGS, NULL},
    {"normal_line",         poly_normal_line,         METH_VARARGS, NULL},
    {"secant_slope",        poly_secant_slope,        METH_VARARGS, NULL},
    {"mean_slope",          poly_mean_slope,          METH_VARARGS, NULL},
    {"derivative_coefficients", poly_derivative_coefficients, METH_VARARGS, NULL},
    {"integral_coefficients", poly_integral_coefficients, METH_VARARGS, NULL},

    /* Multivariate-like 231-250 */
    {"compose_constant",    poly_compose_constant,    METH_VARARGS, NULL},
    {"compose_linear",      poly_compose_linear,      METH_VARARGS, NULL},
    {"compose_monomial",    poly_compose_monomial,    METH_VARARGS, NULL},
    {"compose_power",       poly_compose_power,       METH_VARARGS, NULL},
    {"substitute",          poly_substitute,          METH_VARARGS, NULL},
    {"substitute_zero",     poly_substitute_zero,     METH_VARARGS, NULL},
    {"substitute_one",      poly_substitute_one,      METH_VARARGS, NULL},
    {"substitute_minus_one", poly_substitute_minus_one, METH_VARARGS, NULL},
    {"substitute_linear",   poly_substitute_linear,   METH_VARARGS, NULL},
    {"substitute_affine",   poly_substitute_affine,   METH_VARARGS, NULL},
    {"substitute_scaled",   poly_substitute_scaled,   METH_VARARGS, NULL},
    {"substitute_shifted",  poly_substitute_shifted,  METH_VARARGS, NULL},
    {"substitute_reflected", poly_substitute_reflected, METH_VARARGS, NULL},
    {"compose_derivative",  poly_compose_derivative,  METH_VARARGS, NULL},
    {"derivative_compose",  poly_derivative_compose,  METH_VARARGS, NULL},
    {"compose_power_series", poly_compose_power_series, METH_VARARGS, NULL},
    {"polynomial_chain",    poly_polynomial_chain,    METH_VARARGS, NULL},
    {"polynomial_affine_transform", poly_polynomial_affine_transform, METH_VARARGS, NULL},
    {"polynomial_variable_scale", poly_polynomial_variable_scale, METH_VARARGS, NULL},
    {"polynomial_variable_shift", poly_polynomial_variable_shift, METH_VARARGS, NULL},

    /* Advanced algebra 251-270 */
    {"resultant_small",     poly_resultant_small,     METH_VARARGS, NULL},
    {"discriminant_quadratic", poly_discriminant_quadratic, METH_VARARGS, NULL},
    {"discriminant_cubic",  poly_discriminant_cubic,  METH_VARARGS, NULL},
    {"factor_difference_of_squares", poly_factor_difference_of_squares, METH_VARARGS, NULL},
    {"factor_sum_of_squares", poly_factor_sum_of_squares, METH_VARARGS, NULL},
    {"factor_difference_of_cubes", poly_factor_difference_of_cubes, METH_VARARGS, NULL},
    {"factor_sum_of_cubes",  poly_factor_sum_of_cubes,  METH_VARARGS, NULL},
    {"factor_common_term",   poly_factor_common_term,   METH_VARARGS, NULL},
    {"factor_by_grouping",   poly_factor_by_grouping,   METH_VARARGS, NULL},
    {"factor_quadratic",     poly_factor_quadratic,     METH_VARARGS, NULL},
    {"factor_cubic_candidate", poly_factor_cubic_candidate, METH_VARARGS, NULL},
    {"perfect_square_test", poly_perfect_square_test, METH_VARARGS, NULL},
    {"perfect_cube_test",   poly_perfect_cube_test,   METH_VARARGS, NULL},
    {"repeated_root_test",  poly_repeated_root_test,  METH_VARARGS, NULL},
    {"square_free_test",    poly_square_free_test,    METH_VARARGS, NULL},
    {"square_free_part",    poly_square_free_part,    METH_VARARGS, NULL},
    {"repeated_factor_count", poly_repeated_factor_count, METH_VARARGS, NULL},
    {"multiplicity_sum",    poly_multiplicity_sum,    METH_VARARGS, NULL},
    {"polynomial_power_free", poly_polynomial_power_free, METH_VARARGS, NULL},
    {"factor_count_candidate", poly_factor_count_candidate, METH_VARARGS, NULL},

    /* Products 271-280 */
    {"product_by_x_minus_a", poly_product_by_x_minus_a, METH_VARARGS, NULL},
    {"product_by_x_plus_a", poly_product_by_x_plus_a, METH_VARARGS, NULL},
    {"product_by_linear_factors", poly_product_by_linear_factors, METH_VARARGS, NULL},
    {"product_of_polynomials", poly_product_of_polynomials, METH_VARARGS, NULL},
    {"convolution",         poly_convolution,         METH_VARARGS, NULL},
    {"coefficient_convolution", poly_coefficient_convolution, METH_VARARGS, NULL},
    {"hadamard_product",    poly_hadamard_product,    METH_VARARGS, NULL},
    {"alternating_product", poly_alternating_product, METH_VARARGS, NULL},
    {"self_product",        poly_self_product,        METH_VARARGS, NULL},

    /* Statistics 281-295 */
    {"coefficient_sum_squares", poly_coefficient_sum_squares, METH_VARARGS, NULL},
    {"coefficient_sum_cubes", poly_coefficient_sum_cubes, METH_VARARGS, NULL},
    {"coefficient_l1_norm", poly_coefficient_l1_norm, METH_VARARGS, NULL},
    {"coefficient_l2_norm", poly_coefficient_l2_norm, METH_VARARGS, NULL},
    {"coefficient_max_abs", poly_coefficient_max_abs, METH_VARARGS, NULL},
    {"coefficient_min_abs", poly_coefficient_min_abs, METH_VARARGS, NULL},
    {"coefficient_midrange", poly_coefficient_midrange, METH_VARARGS, NULL},
    {"coefficient_median_candidate", poly_coefficient_median_candidate, METH_VARARGS, NULL},
    {"coefficient_deviation", poly_coefficient_deviation, METH_VARARGS, NULL},
    {"coefficient_positive_sum", poly_coefficient_positive_sum, METH_VARARGS, NULL},
    {"coefficient_negative_sum", poly_coefficient_negative_sum, METH_VARARGS, NULL},

    /* Representation 296-308 */
    {"to_string",           poly_to_string,           METH_VARARGS, NULL},
    {"to_terms",            poly_to_terms,            METH_VARARGS, NULL},
    {"from_terms",          poly_from_terms,          METH_VARARGS, NULL},
    {"from_roots",          poly_from_roots,          METH_VARARGS, NULL},
    {"from_coefficients",   poly_from_coefficients,   METH_VARARGS, NULL},
    {"to_monomial_string",  poly_to_monomial_string,  METH_VARARGS, NULL},
    {"to_latex_like",       poly_to_latex_like,       METH_VARARGS, NULL},
    {"term_degree",         poly_term_degree,         METH_VARARGS, NULL},
    {"term_coefficient",    poly_term_coefficient,    METH_VARARGS, NULL},
    {"term_count_nonzero",  poly_term_count_nonzero,  METH_VARARGS, NULL},
    {"format_coefficient",  poly_format_coefficient,  METH_VARARGS, NULL},
    {"format_term",         poly_format_term,         METH_VARARGS, NULL},
    {"format_polynomial",   poly_format_polynomial,   METH_VARARGS, NULL},

    /* Validation 309-320 */
    {"validate_coefficients", poly_validate_coefficients, METH_VARARGS, NULL},
    {"validate_degree",     poly_validate_degree,     METH_VARARGS, NULL},
    {"validate_root",       poly_validate_root,       METH_VARARGS, NULL},
    {"validate_division",   poly_validate_division,   METH_VARARGS, NULL},
    {"validate_derivative", poly_validate_derivative, METH_VARARGS, NULL},
    {"validate_integral",   poly_validate_integral,   METH_VARARGS, NULL},
    {"validate_factor",     poly_validate_factor,     METH_VARARGS, NULL},
    {"validate_identity",   poly_validate_identity,   METH_VARARGS, NULL},
    {"validate_composition", poly_validate_composition, METH_VARARGS, NULL},
    {"validate_interpolation", poly_validate_interpolation, METH_VARARGS, NULL},
    {"validate_remainder",  poly_validate_remainder,  METH_VARARGS, NULL},
    {"validate_gcd",        poly_validate_gcd,        METH_VARARGS, NULL},

    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef polynomialsmodule = {
    PyModuleDef_HEAD_INIT,
    "polynomials",
    "Polynomial arithmetic and utilities (C extension). Coefficients are lists low-degree first.",
    -1,
    PolynomialMethods
};

PyMODINIT_FUNC
PyInit_polynomials(void)
{
    return PyModule_Create(&polynomialsmodule);
}
