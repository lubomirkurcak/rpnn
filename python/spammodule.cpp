#define PY_SSIZE_T_CLEAN
#include "Python.h"

#include "shared.h"
#include "idx.h"
#include "cpfloat_binary64.h"
#include "neural_network.h"
#include "neural_network.cpp"




/*






               IMPORTANT NOTE(lubo):


                      If you want to debug C code called from python, you CAN NOT USE WANDB, the breakpoints will not be hit!!!
					  
                      Instead, call the module functions directly.







*/



//#undef true
//#undef false

// TODO(lubo): This is not very clean!
global buffer global_debug_mnist_test_bitmap_buffer;
global Loaded_Bitmap global_debug_mnist_test_bitmap;

// TODO(lubo): Artificially expanding the training data regularization
internal int neural_net_test(
    Neural_Network_Hyperparams *params,
    int layer_count, int *layer_sizes,
    int epochs = 30,
    int minibatch_size = 10)
{
    buffer train_set_file = read_entire_file(String("data\\mnist\\train-images.idx3-ubyte"));
    buffer train_labels_file = read_entire_file(String("data\\mnist\\train-labels.idx1-ubyte"));
    buffer test_set_file = read_entire_file(String("data\\mnist\\t10k-images.idx3-ubyte"));
    buffer test_labels_file = read_entire_file(String("data\\mnist\\t10k-labels.idx1-ubyte"));
    
    LK_IDX_File train_set = process_idx_file(train_set_file);
    LK_IDX_File train_labels = process_idx_file(train_labels_file);
    LK_IDX_File test_set = process_idx_file(test_set_file);
    LK_IDX_File test_labels = process_idx_file(test_labels_file);

    // NOTE(lubo): Load one of the pictures into displayable texture
    {
        int display_element_id = 123;
        void *display_element_data = idx_get_element(train_set, display_element_id);
    
        global_debug_mnist_test_bitmap.width = train_set.dimension_sizes[1];
        global_debug_mnist_test_bitmap.height = train_set.dimension_sizes[2];
        global_debug_mnist_test_bitmap.channels = 4;
        global_debug_mnist_test_bitmap_buffer = general_allocate(global_debug_mnist_test_bitmap.width*global_debug_mnist_test_bitmap.height*global_debug_mnist_test_bitmap.channels);
        global_debug_mnist_test_bitmap.data = global_debug_mnist_test_bitmap_buffer.memory;
    
        u32 *write = (u32 *)global_debug_mnist_test_bitmap.data;
        for(int y=0; y<global_debug_mnist_test_bitmap.height; ++y)
        {
            u8 *read = (u8 *)display_element_data + (global_debug_mnist_test_bitmap.height-y-1)*train_set.type_size*global_debug_mnist_test_bitmap.width;
            for(int x=0; x<global_debug_mnist_test_bitmap.width; ++x)
            {
                u32 value = *read;
                #if 0
                *write = (value<<24 | value<<16 | value<<8 | value<<0);
                #else
                *write = (255<<24 | value<<16 | value<<8 | value<<0);
                #endif

                read++;
                write++;
            }
        }
    }
    
    Neural_Network _net = create_feedforward_net(params, layer_count, layer_sizes);
    Neural_Network *net = &_net;

    //save(net);
    
    int correctly_classified_after_training = train_idx(net,
              train_set, train_labels,
              test_set, test_labels,
              epochs, minibatch_size);

    incinerate(net);
              
    return correctly_classified_after_training;
}

static PyObject *SpamError;

static PyObject *
spam_system(PyObject *self, PyObject *args)
{
    const char *command;
    int sts;

    if (!PyArg_ParseTuple(args, "s", &command))
        return NULL;
    sts = system(command);
    if (sts != 0) {
        PyErr_SetString(SpamError, "System command failed");
        return NULL;
    }
    
    return PyLong_FromLong(sts);
}

static PyObject *
spam_reduce_precision(PyObject *self, PyObject *args)
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
		PyErr_SetString(SpamError, "Invalid subnormal argument");//("Invalid subnormal argument: %s\n", subnormal_enum);
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
		PyErr_SetString(SpamError, "Invalid round argument");//PySys_WriteStdout("Invalid round argument: %s\n", round_enum);
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
		PyErr_SetString(SpamError, "Invalid flip argument");//PySys_WriteStdout("Invalid flip argument: %s\n", flip_enum);
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
		PyErr_SetString(SpamError, "Invalid explim argument");//PySys_WriteStdout("Invalid explim argument: %s\n", explim_enum);
		return false;
	}
	
	return true;
}

static PyObject *
spam_matrix_factorization(PyObject *self, PyObject *args, PyObject *keywds)
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
		"dropout",
		"momentum_coefficient",
		"fp_precision",
		"fp_emax",
		"fp_subnormal",
		"fp_round",
		"fp_flip",
		"fp_p",
		"fp_explim",
		NULL};
		
	if (!PyArg_ParseTupleAndKeywords(args, keywds, "|iiiddddiisssfs:matrix_factorization", kwlist, 
		&features,
		&epochs, 
		&non_negative,
		&params.learning_rate,
		&params.weight_decay,
		&params.dropout,
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
spam_mlp_classifier(PyObject *self, PyObject *args, PyObject *keywds)
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
		"dropout",
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
		&params.dropout,
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
		PyErr_SetString(SpamError, "Layer sizes not provided");
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
        neural_net_test(&params, layer_count, layer_sizes, epochs, minibatch_size);
	
    free(layer_sizes);
    
    return PyLong_FromLong(correctly_classified_after_training);
}

static PyMethodDef SpamMethods[] = {
    
    {"system",  spam_system, METH_VARARGS, "Execute a shell command."},
    {"reduce_precision",  spam_reduce_precision, METH_VARARGS, "reduce_precision"},
    //{"create_forward_net",  create_forward_net, METH_VARARGS, "create_forward_net"},
    {"mlp_classifier",  (PyCFunction)spam_mlp_classifier, METH_VARARGS | METH_KEYWORDS, "mlp_classifier"},
    {"matrix_factorization",  (PyCFunction)spam_matrix_factorization, METH_VARARGS | METH_KEYWORDS, "matrix_factorization"},

    {NULL, NULL, 0, NULL}        /* Sentinel */
};

static struct PyModuleDef spammodule = {
    PyModuleDef_HEAD_INIT,
    "spam",   /* name of module */
    0, /* module documentation, may be NULL */
    -1,       /* size of per-interpreter state of the module,
                 or -1 if the module keeps state in global variables. */
    SpamMethods
};

PyMODINIT_FUNC
PyInit_spam(void)
{
    PyObject *m;

    m = PyModule_Create(&spammodule);
    if (m == NULL)
        return NULL;

    SpamError = PyErr_NewException("spam.error", NULL, NULL);
    Py_XINCREF(SpamError);
    if (PyModule_AddObject(m, "error", SpamError) < 0) {
        Py_XDECREF(SpamError);
        Py_CLEAR(SpamError);
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
    if (PyImport_AppendInittab("spam", PyInit_spam) == -1) {
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
    PyObject *pmodule = PyImport_ImportModule("spam");
    if (!pmodule) {
        PyErr_Print();
        fprintf(stderr, "Error: could not import module 'spam'\n");
    }

    //...

    PyMem_RawFree(program);
    return 0;
}

