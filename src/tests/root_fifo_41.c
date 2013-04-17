// test txn commit after db close

#include "test.h"
#include <sys/stat.h>

DB_ENV *null_env = NULL;
DB *null_db = NULL;
DB_TXN *null_txn = NULL;
DBC *null_cursor = NULL;

static void create_non_empty(int n) {
    DB_ENV *env = null_env;
    int r;
    r = db_env_create(&env, 0); assert(r == 0); assert(env != NULL);
    r = env->open(env, 
                  ENVDIR, 
                  DB_INIT_MPOOL+DB_INIT_LOG+DB_INIT_LOCK+DB_INIT_TXN+DB_PRIVATE+DB_CREATE, 
                  S_IRWXU+S_IRWXG+S_IRWXO); 
    assert(r == 0);

    DB_TXN *txn = null_txn;
    r = env->txn_begin(env, null_txn, &txn, 0); assert(r == 0); assert(txn != NULL);

    DB *db = null_db;
    r = db_create(&db, env, 0); assert(r == 0); assert(db != NULL);

    r = db->open(db, txn, "test.db", 0, DB_BTREE, DB_CREATE, S_IRWXU+S_IRWXG+S_IRWXO); 
    assert(r == 0);

    int i;
    for (i=n; i<2*n; i++) {
        DBT key, val;
        int k = toku_htonl(i);
        r = db->put(db, txn, dbt_init(&key, &k, sizeof k), dbt_init(&val, &i, sizeof i), 0);
        assert(r == 0);
    }

    r = db->close(db, 0); assert(r == 0); db = null_db;

    r = txn->commit(txn, 0); assert(r == 0); txn = null_txn;

    r = env->close(env, 0); assert(r == 0); env = null_env;
}

static void root_fifo_verify(DB_ENV *env, int n, int expectn) {
    if (verbose) printf("%s:%d %d %d\n", __FUNCTION__, __LINE__, n, expectn);

    int r;
    DB_TXN *txn = null_txn;
    r = env->txn_begin(env, null_txn, &txn, 0); assert(r == 0); assert(txn != NULL);

    DB *db = null_db;
    r = db_create(&db, env, 0); assert(r == 0); assert(db != NULL);
    r = db->open(db, txn, "test.db", 0, DB_BTREE, DB_CREATE, S_IRWXU+S_IRWXG+S_IRWXO); 
    assert(r == 0);

    DBC *cursor = null_cursor;
    r = db->cursor(db, txn, &cursor, 0); assert(r == 0);
    int i;
    for (i = 0; ; i++) {
        DBT key, val;
        memset(&key, 0, sizeof key); memset(&val, 0, sizeof val);
        r = cursor->c_get(cursor, &key, &val, DB_NEXT);
        if (r != 0) break;
        int k;
        assert(key.size == sizeof k);
        memcpy(&k, key.data, key.size);
        assert((int)toku_ntohl(k) == i);
    }
    assert(i == expectn);

    r = cursor->c_close(cursor); assert(r == 0); cursor = null_cursor;
    
    r = txn->commit(txn, 0); assert(r == 0); txn = null_txn;

    r = db->close(db, 0); assert(r == 0); db = null_db;
}

static void root_fifo_41(int n, int ntxn, BOOL do_populate) {
    if (verbose) printf("%s:%d %d\n", __FUNCTION__, __LINE__, n);
    int r;

    // create the env
    system("rm -rf " ENVDIR);
    toku_os_mkdir(ENVDIR, S_IRWXU+S_IRWXG+S_IRWXO);

    // populate
    if (do_populate)
        create_non_empty(n);

    DB_ENV *env = null_env;
    r = db_env_create(&env, 0); assert(r == 0); assert(env != NULL);
    r = env->open(env, 
                  ENVDIR, 
                  DB_INIT_MPOOL+DB_INIT_LOG+DB_INIT_LOCK+DB_INIT_TXN+DB_PRIVATE+DB_CREATE, 
                  S_IRWXU+S_IRWXG+S_IRWXO); 
    assert(r == 0);
    {
        DB_TXN *txn;
        DB *db = null_db;
        r = env->txn_begin(env, null_txn, &txn, 0); CKERR(r);
        r = db_create(&db, env, 0); CKERR(r);
        r = db->open(db, txn, "test.db", 0, DB_BTREE, DB_CREATE, S_IRWXU+S_IRWXG+S_IRWXO); 
        CKERR(r);
        r = txn->commit(txn, 0); CKERR(r);
        r = db->close(db, 0); CKERR(r);
    }

    DB_TXN *txn[ntxn];
    int i;
    for (i=0; i<ntxn; i++) {
        r = env->txn_begin(env, null_txn, &txn[i], 0); assert(r == 0); assert(txn[i] != NULL);
    }

    for (i=0; i<n; i++) {
        DB *db = null_db;
        r = db_create(&db, env, 0); assert(r == 0); assert(db != NULL);

        r = db->open(db, txn[i % ntxn], "test.db", 0, DB_BTREE, DB_CREATE, S_IRWXU+S_IRWXG+S_IRWXO); 
        assert(r == 0);

        DBT key, val;
        int k = toku_htonl(i);
        r = db->put(db, txn[i % ntxn], dbt_init(&key, &k, sizeof k), dbt_init(&val, &i, sizeof i), 0);
        assert(r == 0);

        r = db->close(db, 0); assert(r == 0); db = null_db;
    }

    for (i=0; i<ntxn; i++) {
        r = txn[i]->commit(txn[i], 0); assert(r == 0);
    }

    // verify the db
    root_fifo_verify(env, n, do_populate ? 2*n : n);

    // cleanup
    r = env->close(env, 0); assert(r == 0); env = null_env;
}

int test_main(int argc, char *argv[]) {
    int i;
    int n = -1;
    int ntxn = -1;
    BOOL do_populate = FALSE;

    // parse_args(argc, argv);
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0) {
            verbose = 1;
            continue;
        }
        if (strcmp(argv[i], "-n") == 0) {
            if (i+1 < argc)
                n = atoi(argv[++i]);
            continue;
        }
        if (strcmp(argv[i], "-ntxn") == 0) {
            if (i+1 < argc)
                ntxn = atoi(argv[++i]);
            continue;
        }
        if (strcmp(argv[i], "-populate") == 0) {
            do_populate = TRUE;
            continue;
        }
    }
              
    if (n >= 0)
        root_fifo_41(n, ntxn == -1 ? 1 : ntxn, do_populate);
    else {
        for (i=0; i<100; i++) {
            for (ntxn=1; ntxn<=4; ntxn++) {
                root_fifo_41(i, ntxn, FALSE);
                root_fifo_41(i, ntxn, TRUE);
            }
        }
    }
    return 0;
}

