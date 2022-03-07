#define PY_SSIZE_T_CLEAN
#include "Python.h"

#include "shared.h"
#include "idx.h"
#include "cpfloat_binary64.h"
#include "matrix.h"
#include "matrix.cpp"
#include "function_approximator.h"
#include "neural_network.h"
#include "neural_network.cpp"
#include "lookup_table.h"
#include "lookup_table.cpp"


#include "test_mnist.cpp"
#include "test_cartpole.cpp"
#include "test_functionapprox.cpp"


/*






               IMPORTANT NOTE(lubo):


                      If you want to debug C code called from python, you CAN NOT USE WANDB, the breakpoints will not be hit!!!
					  
                      Instead, call the module functions directly.







*/


/*

TODOs(lubo):

   Interface using Numpy?

*/

// TODO(lubo): This is not very clean!
global buffer global_debug_mnist_test_bitmap_buffer;
global Loaded_Bitmap global_debug_mnist_test_bitmap;

static PyObject *RPNNError;

static PyObject *
rpnn_system(PyObject *self, PyObject *args)
{
    const char *command;
    int sts;

    if (!PyArg_ParseTuple(args, "s", &command))
        return NULL;
    sts = system(command);
    if (sts != 0) {
        PyErr_SetString(RPNNError, "System command failed");
        return NULL;
    }
    
    return PyLong_FromLong(sts);
}

static PyObject *
rpnn_reduce_precision(PyObject *self, PyObject *args)
{
    float value;
    float precision = 10;

    if (!PyArg_ParseTuple(args, "f|f", &value, &precision))
        return NULL;
    
    return PyFloat_FromDouble((float64)value*precision);
}

static bool set_fpopts_by_names(optstruct *fpopts,
    char *subnormal_enum,
	char *round_enum,
	char *flip_enum,
	char *explim_enum)
{
	if(strcmp(subnormal_enum, "CPFLOAT_SUBN_RND") == 0)
	{
		fpopts->subnormal = CPFLOAT_SUBN_RND;
	}
	else if(strcmp(subnormal_enum, "CPFLOAT_SUBN_USE") == 0)
	{
		fpopts->subnormal = CPFLOAT_SUBN_USE;
	}
	else
	{
		PyErr_SetString(RPNNError, "Invalid subnormal argument");//("Invalid subnormal argument: %s\n", subnormal_enum);
		return false;
	}
	
	if(strcmp(round_enum, "CPFLOAT_RND_NA") == 0)
	{
		fpopts->round = CPFLOAT_RND_NA;
	}
	else if(strcmp(round_enum, "CPFLOAT_RND_NZ") == 0)
	{
		fpopts->round = CPFLOAT_RND_NZ;
	}
	else if(strcmp(round_enum, "CPFLOAT_RND_NE") == 0)
	{
		fpopts->round = CPFLOAT_RND_NE;
	}
	else if(strcmp(round_enum, "CPFLOAT_RND_TP") == 0)
	{
		fpopts->round = CPFLOAT_RND_TP;
	}
	else if(strcmp(round_enum, "CPFLOAT_RND_TN") == 0)
	{
		fpopts->round = CPFLOAT_RND_TN;
	}
	else if(strcmp(round_enum, "CPFLOAT_RND_TZ") == 0)
	{
		fpopts->round = CPFLOAT_RND_TZ;
	}
	else if(strcmp(round_enum, "CPFLOAT_RND_SP") == 0)
	{
		fpopts->round = CPFLOAT_RND_SP;
	}
	else if(strcmp(round_enum, "CPFLOAT_RND_SE") == 0)
	{
		fpopts->round = CPFLOAT_RND_SE;
	}
	else if(strcmp(round_enum, "CPFLOAT_RND_OD") == 0)
	{
		fpopts->round = CPFLOAT_RND_OD;
	}
	else if(strcmp(round_enum, "CPFLOAT_NO_RND") == 0)
	{
		fpopts->round = CPFLOAT_NO_RND;
	}
	else
	{
		PyErr_SetString(RPNNError, "Invalid round argument");//PySys_WriteStdout("Invalid round argument: %s\n", round_enum);
		return false;
	}
	
	if(strcmp(flip_enum, "CPFLOAT_NO_SOFTERR") == 0)
	{
		fpopts->flip = CPFLOAT_NO_SOFTERR;
	}
	else if(strcmp(flip_enum, "CPFLOAT_SOFTERR") == 0)
	{
		fpopts->flip = CPFLOAT_SOFTERR;
	}
	else
	{
		PyErr_SetString(RPNNError, "Invalid flip argument");//PySys_WriteStdout("Invalid flip argument: %s\n", flip_enum);
		return false;
	}
	
	if(strcmp(explim_enum, "CPFLOAT_EXPRANGE_STOR") == 0)
	{
		fpopts->explim = CPFLOAT_EXPRANGE_STOR;
	}
	else if(strcmp(explim_enum, "CPFLOAT_EXPRANGE_TARG") == 0)
	{
		fpopts->explim = CPFLOAT_EXPRANGE_TARG;
	}
	else
	{
		PyErr_SetString(RPNNError, "Invalid explim argument");//PySys_WriteStdout("Invalid explim argument: %s\n", explim_enum);
		return false;
	}
	
	return true;
}

static PyObject *
rpnn_matrix_factorization(PyObject *self, PyObject *args, PyObject *keywds)
{
    Neural_Network_Hyperparams params = default_hyperparams();
	
	int features = 10;
	int epochs = 30;
	int non_negative = true;
	
    optstruct fpopts = precision_full();
  
    char *subnormal_enum = "CPFLOAT_SUBN_USE";
	char *round_enum = "CPFLOAT_RND_TP";
	char *flip_enum = "CPFLOAT_NO_SOFTERR";
	char *explim_enum = "CPFLOAT_EXPRANGE_TARG";
	
	static char *kwlist[] = {
		"features",
		"epochs",
		"non_negative",
		"learning_rate",
		"weight_decay",
		"dropout_keep_p",
		"momentum_coefficient",
		"fp_precision",
		"fp_emax",
		"fp_subnormal",
		"fp_round",
		"fp_flip",
		"fp_p",
		"fp_explim",
		NULL};
		
	if (!PyArg_ParseTupleAndKeywords(args, keywds, "|iipddddiisssfs:matrix_factorization", kwlist, 
		&features,
		&epochs, 
		&non_negative,
		&params.learning_rate,
		&params.weight_decay,
		&params.dropout_keep_p,
		&params.momentum_coefficient,
		&fpopts.precision,
		&fpopts.emax,
		&subnormal_enum,
		&round_enum,
		&flip_enum,
		&fpopts.p,
		&explim_enum)) 
	{
		return NULL;
	}

	if(!set_fpopts_by_names(&fpopts, subnormal_enum, round_enum, flip_enum, explim_enum)) return NULL;
    PySys_WriteStdout("cpfloat_validate_optstruct returned %d\n", cpfloat_validate_optstruct(&fpopts));
	
    params.forward_fpopts = fpopts;
    params.backprop_fpopts = fpopts;
    params.update_fpopts = fpopts;
	
    // NOTE(lubo): Uniformly random integers from [1, 5]
    double _C[] =
    {
        5, 2, 5, 4, 1, 5, 5, 3, 1, 3, 2, 4, 5, 2, 3, 1, 4, 3, 5, 3, 1, 2, 5, 4, 3, 5, 5, 3, 3, 1, 2, 1, 4, 5, 5, 1, 2, 3, 1, 5, 1, 3, 2, 1, 2, 3, 3, 2, 2, 1,
        4, 5, 4, 1, 4, 1, 3, 4, 3, 4, 2, 2, 4, 2, 1, 3, 2, 3, 1, 2, 2, 1, 3, 5, 3, 2, 2, 5, 2, 1, 5, 5, 2, 3, 1, 3, 1, 2, 5, 1, 1, 1, 4, 1, 2, 3, 5, 3, 5, 2,
        1, 3, 5, 3, 4, 3, 5, 5, 1, 5, 5, 5, 5, 1, 5, 3, 2, 3, 1, 5, 4, 4, 3, 3, 1, 4, 1, 2, 4, 5, 1, 3, 3, 2, 3, 5, 5, 4, 4, 1, 5, 1, 4, 5, 2, 4, 5, 4, 5, 4,
        5, 2, 2, 3, 3, 5, 2, 3, 4, 5, 1, 4, 3, 2, 2, 4, 4, 3, 4, 3, 2, 1, 2, 5, 4, 1, 4, 5, 5, 1, 2, 4, 2, 5, 3, 1, 3, 4, 3, 4, 2, 5, 2, 4, 3, 3, 4, 5, 1, 1,
        2, 5, 1, 1, 1, 2, 2, 2, 1, 2, 1, 4, 5, 5, 4, 2, 4, 1, 5, 3, 3, 4, 1, 4, 1, 5, 4, 3, 5, 1, 3, 4, 2, 1, 5, 1, 1, 2, 1, 5, 4, 4, 4, 2, 1, 2, 1, 2, 3, 5,
        2, 5, 5, 5, 5, 5, 2, 2, 3, 5, 3, 2, 3, 5, 5, 3, 3, 3, 2, 4, 3, 2, 4, 5, 1, 4, 2, 1, 5, 2, 4, 3, 3, 4, 5, 2, 5, 3, 3, 3, 3, 2, 3, 2, 4, 3, 3, 3, 4, 2,
        5, 3, 1, 3, 2, 1, 4, 5, 4, 5, 4, 2, 4, 3, 2, 1, 4, 5, 4, 3, 1, 5, 1, 2, 4, 2, 1, 3, 2, 5, 4, 5, 3, 5, 1, 2, 3, 1, 4, 5, 5, 1, 1, 3, 2, 5, 2, 1, 1, 1,
        2, 4, 4, 1, 5, 1, 4, 4, 1, 5, 3, 3, 4, 3, 1, 3, 4, 1, 2, 5, 3, 4, 5, 5, 4, 3, 3, 5, 1, 5, 4, 4, 3, 4, 5, 5, 4, 1, 4, 4, 3, 2, 3, 5, 5, 5, 3, 3, 1, 4,
        3, 1, 2, 5, 2, 3, 1, 2, 1, 3, 5, 2, 2, 1, 5, 2, 1, 3, 2, 1, 4, 3, 2, 5, 3, 3, 5, 2, 3, 5, 4, 3, 3, 5, 5, 5, 5, 4, 1, 3, 2, 4, 1, 1, 4, 3, 1, 5, 1, 2,
        5, 5, 3, 5, 4, 3, 1, 1, 2, 2, 3, 2, 4, 4, 4, 5, 3, 1, 3, 4, 3, 4, 4, 5, 1, 5, 5, 1, 2, 3, 5, 5, 3, 5, 4, 5, 3, 3, 1, 1, 3, 2, 4, 3, 1, 3, 1, 5, 5, 2,
        5, 3, 3, 4, 3, 2, 2, 5, 4, 5, 4, 4, 3, 1, 2, 3, 5, 5, 3, 5, 1, 1, 4, 1, 1, 2, 3, 3, 5, 3, 5, 5, 1, 5, 2, 3, 1, 5, 1, 5, 5, 5, 1, 4, 5, 5, 2, 5, 1, 4,
        4, 5, 2, 2, 3, 1, 1, 4, 1, 2, 5, 3, 3, 2, 4, 2, 3, 4, 5, 2, 5, 3, 4, 5, 3, 5, 5, 1, 4, 4, 1, 2, 5, 3, 1, 3, 2, 2, 5, 3, 1, 5, 3, 1, 5, 4, 1, 2, 3, 4,
        1, 5, 2, 5, 5, 4, 4, 4, 3, 2, 2, 4, 1, 1, 1, 5, 4, 2, 2, 5, 5, 4, 1, 5, 1, 5, 1, 4, 4, 1, 4, 2, 4, 3, 5, 5, 2, 4, 3, 2, 5, 1, 1, 4, 1, 3, 5, 5, 3, 4,
        5, 3, 4, 5, 4, 4, 3, 1, 5, 5, 4, 4, 3, 4, 5, 2, 4, 5, 5, 2, 2, 3, 1, 3, 1, 3, 1, 5, 3, 1, 1, 4, 5, 5, 4, 4, 3, 5, 1, 2, 1, 1, 4, 2, 3, 2, 1, 1, 3, 3,
        5, 5, 1, 5, 3, 1, 3, 1, 2, 3, 5, 5, 2, 5, 5, 3, 4, 2, 5, 5, 1, 5, 3, 1, 5, 2, 4, 5, 2, 3, 1, 2, 1, 3, 4, 2, 2, 3, 3, 3, 4, 5, 4, 2, 4, 4, 3, 2, 4, 3,
        4, 3, 1, 3, 3, 5, 5, 5, 2, 3, 3, 5, 2, 2, 2, 3, 4, 2, 4, 2, 4, 1, 2, 4, 5, 5, 1, 2, 5, 2, 5, 3, 2, 5, 4, 1, 5, 1, 2, 1, 1, 1, 5, 3, 5, 1, 3, 2, 2, 2,
        2, 1, 5, 2, 5, 5, 3, 1, 2, 5, 3, 3, 5, 2, 4, 1, 2, 3, 3, 3, 2, 5, 5, 5, 2, 1, 4, 3, 4, 1, 4, 3, 4, 5, 5, 4, 3, 2, 5, 5, 2, 5, 3, 5, 3, 4, 3, 3, 1, 2,
        1, 1, 5, 5, 5, 3, 2, 2, 3, 5, 5, 4, 5, 4, 5, 4, 2, 3, 2, 2, 5, 2, 1, 4, 4, 1, 3, 3, 4, 3, 5, 4, 1, 5, 4, 3, 4, 3, 1, 2, 1, 4, 2, 2, 1, 4, 1, 4, 4, 2,
        3, 1, 1, 4, 1, 1, 1, 4, 1, 4, 4, 1, 4, 5, 4, 5, 4, 5, 1, 3, 1, 5, 4, 1, 5, 1, 4, 3, 3, 4, 4, 1, 4, 3, 5, 2, 4, 3, 4, 4, 5, 4, 1, 1, 2, 2, 2, 5, 5, 1,
        2, 1, 1, 5, 2, 1, 1, 4, 1, 2, 1, 4, 2, 5, 4, 4, 1, 2, 5, 4, 3, 1, 1, 3, 2, 1, 3, 2, 5, 4, 2, 2, 5, 1, 2, 2, 3, 2, 2, 3, 3, 5, 1, 3, 4, 5, 2, 3, 1, 1,
    };
    matrix C = {20,50,1,1};
    C.allocation.memory = (u8 *)_C;
    C.allocation.size = sizeof(_C);

    float64 final_cost = matrix_factorization(&params, C, features, epochs, (b32x)non_negative);
    
    return PyFloat_FromDouble(final_cost);
}

static PyObject *
rpnn_mlp_classifier(PyObject *self, PyObject *args, PyObject *keywds)
{
    Neural_Network_Hyperparams params = default_hyperparams();
	
    PyObject *layer_sizes_tuple;
	
	int epochs = 30;
    int minibatch_size = 10;
	
    optstruct fpopts = precision_full();
  
    char *subnormal_enum = "CPFLOAT_SUBN_USE";
	char *round_enum = "CPFLOAT_RND_TP";
	char *flip_enum = "CPFLOAT_NO_SOFTERR";
	char *explim_enum = "CPFLOAT_EXPRANGE_TARG";
	
	static char *kwlist[] = {
		"layer_sizes",
		"epochs",
		"minibatch_size",
		"learning_rate",
		"weight_decay",
		"dropout_keep_p",
		"momentum_coefficient",
		"fp_precision",
		"fp_emax",
		"fp_subnormal",
		"fp_round",
		"fp_flip",
		"fp_p",
		"fp_explim",
		NULL};
		
	if (!PyArg_ParseTupleAndKeywords(args, keywds, "O|iiddddiisssfs:mlp_classifier", kwlist, 
		&layer_sizes_tuple,
		&epochs, 
		&minibatch_size,
		&params.learning_rate,
		&params.weight_decay,
		&params.dropout_keep_p,
		&params.momentum_coefficient,
		&fpopts.precision,
		&fpopts.emax,
		&subnormal_enum,
		&round_enum,
		&flip_enum,
		&fpopts.p,
		&explim_enum)) 
	{
		return NULL;
	}

	if(!set_fpopts_by_names(&fpopts, subnormal_enum, round_enum, flip_enum, explim_enum)) return NULL;
	PySys_WriteStdout("cpfloat_validate_optstruct returned %d\n", cpfloat_validate_optstruct(&fpopts));
	
    params.forward_fpopts = fpopts;
    params.backprop_fpopts = fpopts;
    params.update_fpopts = fpopts;
	
	if(layer_sizes_tuple == NULL)
	{
		PyErr_SetString(RPNNError, "Layer sizes not provided");
		return NULL;
	}
	
    Py_ssize_t layer_count_Py_ssize_t = PyTuple_Size(layer_sizes_tuple);
    int layer_count = safecast_to_int(layer_count_Py_ssize_t);
    int *layer_sizes = (int *)malloc(layer_count*sizeof(int));
    for(int i=0; i<layer_count; ++i)
    {
        layer_sizes[i] = (int)PyLong_AsLong(PyTuple_GetItem(layer_sizes_tuple, i));
    }
	
    int correctly_classified_after_training = 
        train_mnist(params, layer_count, layer_sizes, epochs, minibatch_size);
	
    free(layer_sizes);
    
    return PyLong_FromLong(correctly_classified_after_training);
}

static bool DQN_CREATED;
static Deep_Q_Network dqn;

static PyObject *
rpnn_cartpole(PyObject *self, PyObject *args, PyObject *keywds)
{
	if(!DQN_CREATED)
	{
		DQN_CREATED = true;
		gym_cartpole_create(&dqn);
	}
	
    PyObject *observation_tuple;
	double reward = 0;
	int done = -1;
	int reset_weights = 0;
	
	static char *kwlist[] = {
		"observation",
		"reward",
		"done",
		"reset_weights",
		NULL};
		
	if (!PyArg_ParseTupleAndKeywords(args, keywds, "O|dpp:cartpole", kwlist, 
		&observation_tuple,
		&reward, 
		&done,
		&reset_weights)) 
	{
		return NULL;
	}

	/* NOTE(lubo): Observations (https://github.com/openai/gym/blob/master/gym/envs/classic_control/cartpole.py)
	| Num | Observation           | Min                  | Max                |
    |-----|-----------------------|----------------------|--------------------|
    | 0   | Cart Position         | -4.8                 | 4.8                |
    | 1   | Cart Velocity         | -Inf                 | Inf                |
    | 2   | Pole Angle            | ~ -0.418 rad (-24°)  | ~ 0.418 rad (24°)  |
    | 3   | Pole Angular Velocity | -Inf                 | Inf                |*/
	double observation[4];
	auto py_list_size = PyList_Size(observation_tuple);
    int observation_list_size = safecast_to_int(py_list_size);
	Assert(observation_list_size == ArrayCount(observation));
    for(int i=0; i<observation_list_size; ++i)
    {
        observation[i] = PyFloat_AsDouble(PyList_GetItem(observation_tuple, i));
    }
	
    /* NOTE(lubo): Actions
	| Num | Action                 |
    |-----|------------------------|
    | 0   | Push cart to the left  |
    | 1   | Push cart to the right |*/
	//int action = probability(0.5f); // NOTE(lubo): Choose random action, Average timesteps: 24.96
    //int action = observation[2] > 0; // NOTE(lubo): Move the way the pole is tilting, Average timesteps: 40.82
    //static Deep_Q_Network dqn = gym_cartpole_create();
	
	if(reset_weights)
	{
		PySys_WriteStdout("Ignoring reset weights.\n");
		//reinitialize_weights_and_biases(&dqn.Q_0);
		//reinitialize_weights_and_biases(&dqn.Q_1);
	}
	
    int action = gym_cartpole_update(&dqn, observation, reward, done);
	
    return PyLong_FromLong(action);
}

static PyMethodDef RPNNMethods[] = {
    
    {"system",  rpnn_system, METH_VARARGS, "Execute a shell command."},
    {"reduce_precision",  rpnn_reduce_precision, METH_VARARGS, "reduce_precision"},
    //{"create_forward_net",  create_forward_net, METH_VARARGS, "create_forward_net"},
    {"mlp_classifier",  (PyCFunction)rpnn_mlp_classifier, METH_VARARGS | METH_KEYWORDS, "mlp_classifier"},
    {"matrix_factorization",  (PyCFunction)rpnn_matrix_factorization, METH_VARARGS | METH_KEYWORDS, "matrix_factorization"},
	{"cartpole",  (PyCFunction)rpnn_cartpole, METH_VARARGS | METH_KEYWORDS, "cartpole"},
	//{"function_approximation",  (PyCFunction)rpnn_function_approximation, METH_VARARGS | METH_KEYWORDS, "function_approximation"},

    {NULL, NULL, 0, NULL}        /* Sentinel */
};

static struct PyModuleDef rpnnmodule = {
    PyModuleDef_HEAD_INIT,
    "rpnn",   /* name of module */
    0, /* module documentation, may be NULL */
    -1,       /* size of per-interpreter state of the module,
                 or -1 if the module keeps state in global variables. */
    RPNNMethods
};

PyMODINIT_FUNC
PyInit_rpnn(void)
{
    PyObject *m;

    m = PyModule_Create(&rpnnmodule);
    if (m == NULL)
        return NULL;

    RPNNError = PyErr_NewException("rpnn.error", NULL, NULL);
    Py_XINCREF(RPNNError);
    if (PyModule_AddObject(m, "error", RPNNError) < 0) {
        Py_XDECREF(RPNNError);
        Py_CLEAR(RPNNError);
        Py_DECREF(m);
        return NULL;
    }

    return m;
}

int
main(int argc, char *argv[])
{
    wchar_t *program = Py_DecodeLocale(argv[0], NULL);
    if (program == NULL) {
        fprintf(stderr, "Fatal error: cannot decode argv[0]\n");
        exit(1);
    }

    /* Add a built-in module, before Py_Initialize */
    if (PyImport_AppendInittab("rpnn", PyInit_rpnn) == -1) {
        fprintf(stderr, "Error: could not extend in-built modules table\n");
        exit(1);
    }

    /* Pass argv[0] to the Python interpreter */
    Py_SetProgramName(program);

    /* Initialize the Python interpreter.  Required.
       If this step fails, it will be a fatal error. */
    Py_Initialize();

    /* Optionally import the module; alternatively,
       import can be deferred until the embedded script
       imports it. */
    PyObject *pmodule = PyImport_ImportModule("rpnn");
    if (!pmodule) {
        PyErr_Print();
        fprintf(stderr, "Error: could not import module 'rpnn'\n");
    }

    //...

    PyMem_RawFree(program);
    return 0;
}

