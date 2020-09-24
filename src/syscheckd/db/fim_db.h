/**
 * @file fim_db.h
 * @brief Definition of FIM database library.
 * @date 2019-08-28
 *
 * @copyright Copyright (c) 2020 Wazuh, Inc.
 */

#ifndef FIM_DB_COMMON_H
#define FIM_DB_COMMON_H

#define fim_db_decode_registry_value_full_row(stmt) _fim_db_decode_registry_value(stmt, 11)

#include "shared.h"
#include <openssl/evp.h>
#include "../syscheck.h"
#include "external/sqlite/sqlite3.h"
#include "config/syscheck-config.h"

#define FIM_DB_MEMORY_PATH  ":memory:"

#ifndef WIN32
#define FIM_DB_DISK_PATH    DEFAULTDIR "/queue/fim/db/fim.db"
#define FIM_DB_TMPDIR       DEFAULTDIR "/tmp/"
#else
#define FIM_DB_DISK_PATH    "queue/fim/db/fim.db"
#define FIM_DB_TMPDIR       "tmp/"
#endif

#define COMMIT_INTERVAL     2

#define FIMDB_OK 0   // Successful result.
#define FIMDB_ERR -1 // Generic error.
#define FIMDB_FULL -2 // DB is full.

#define FIMDB_RM_MAX_LOOP 10 // Max number of loop iterations
#define FIMDB_RM_DEFAULT_TIME 100 //miliseconds

#define FIM_LAST_ROW 0
#define FIM_FIRST_ROW 1

#define EVP_MAX_MD_SIZE 64

#define FIM_DB_PATHS    100

#define FIM_DB_DECODE_TYPE(_func) (void *(*)(sqlite3_stmt *))(_func)
#define FIM_DB_FREE_TYPE(_func) (void (*)(void *))(_func)
#define FIM_DB_CALLBACK_TYPE(_func) (void (*)(fdb_t *, void *, int,  void *))(_func)

extern const char *schema_fim_sql;

/**
 * @brief Executes a simple query in a given database.
 *
 * @param fim_sql The FIM database structure where the database is.
 * @param query The query to be executed.
 *
 * @return int 0 on success, -1 on error.
 */
int fim_db_exec_simple_wquery(fdb_t *fim_sql, const char *query);


/**
 * @brief
 *
 * @param fim_sql FIM database structure.
 * @param type Variable to indicate if the query is for registries or for files. 0 (FIM_TYPE_FILE) for files
 *  1 (FIM_TYPE_REGISTRY) for registries.
 * @param index Statement index.
 * @param callback Callback to be used.
 * @param storage Type of storage (memory or disk).
 * @param arg Storage which contains all the paths
 *
 * @return FIMDB_OK on success, FIMDB_ERR otherwise.
 */
int fim_db_process_get_query(fdb_t *fim_sql, int type, int index, void (*callback)(fdb_t *, fim_entry *, int, void *),
                             int storage, void * arg);

/**
 * @brief
 *
 * @param fim_sql FIM database structure.
 * @param index Statement index.
 * @param decode Decode function to be used.
 * @param free_row Free function to be used.
 * @param callback Callback to be used.
 * @param storage Type of storage (memory or disk).
 * @param arg Storage which contains all the paths.
 *
 * @return FIMDB_OK on success, FIMDB_ERR otherwise.
 */
int fim_db_multiple_row_query(fdb_t *fim_sql, int index, void *(*decode)(sqlite3_stmt *), void (*free_row)(void *),
                              void (*callback)(fdb_t *, void *, int, void *), int storage, void *arg);

/**
 * @brief Create a new database.
 *
 * @param path New database path.
 * @param source SQlite3 schema file.
 * @param storage Type of storage (memory or disk).
 * @param fim_db Database pointer.
 *
 * @return 0 on success, -1 otherwise
 */
int fim_db_create_file(const char *path, const char *source, int storage, sqlite3 **fim_db);

/**
 * @brief Create a new temporal storage to save all the files' paths.
 *
 * @param storage Type of storage (memory or disk).
 *
 * @return New file structure.
 */
fim_tmp_file *fim_db_create_temp_file(int storage);


/**
 * @brief Clean and free resources.
 *
 * @param file Storage structure.
 * @param storage Type of storage (memory or disk).
 */
void fim_db_clean_file(fim_tmp_file **file, int storage);

/**
 * @brief Decodes a row from the registry database to be saved in a registry key structure.
 *
 * @param stmt The statement to be decoded.
 * @param index Index of the statement.
 *
 * @return fim_entry* The filled structure.
 */
fim_entry *fim_db_decode_registry(int index, sqlite3_stmt *stmt);

/**
 * @brief Decodes a row from the database to be saved in a fim_entry structure.
 *
 * @param stmt The statement to be decoded.
 *
 * @return fim_entry* The filled structure.
 */
fim_entry *fim_db_decode_full_row(sqlite3_stmt *stmt);

/**
 * @brief Get entry data using path.
 *
 * @param fim_sql FIM database struct.
 * @param file_path File path.
 *
 * @return FIM entry struct on success, NULL on error.
 */
fim_entry *fim_db_get_path(fdb_t *fim_sql, const char *file_path);

/**
 * @brief Get a registry key using its path.
 *
 * @param fim_sql FIM database struct.
 * @param path Path to registry key.
 *
 * @return FIM registry key struct on success, NULL on error.
*/
fim_registry_key *fim_db_get_registry_key(fdb_t *fim_sql, const char *path);

/**
 * @brief Read paths and registry paths which are stored in a temporal storage.
 *
 * @param fim_sql FIM database structure.
 * @param type Type of entry that will be used. It can be FIM_TYPE_REGISTRY or FIM_TYPE_FILE.
 * @param mutex FIM database's mutex for thread synchronization.
 * @param storage 1 Store database in memory, disk otherwise.
 * @param callback Function to call within a step.
 * @param mode FIM mode for callback function.
 * @param w_evt Whodata information for callback function.
 *
 * @return FIMDB_OK on success, FIMDB_ERR otherwise.
 */
 int fim_db_process_read_file(fdb_t *fim_sql, fim_tmp_file *file, int type, pthread_mutex_t *mutex,
                              void (*callback)(fdb_t *, fim_entry *, pthread_mutex_t *, void *, void *, void *),
                              int storage, void * alert, void * mode, void * w_evt);

/**
 * @brief Initialize FIM databases.
 *
 * Checks if the databases exists.
 * If it exists deletes the previous version and creates a new one.
 *
 * @param storage 1 Store database in memory, disk otherwise.
 *
 * @return FIM database struct.
 */
fdb_t *fim_db_init(int storage);

/**
 * @brief Finalize stmt and close DB.
 *
 * @param fim_sql FIM database struct.
 *
 * @return FIMDB_OK on success, FIMDB_ERR otherwise.
 */
void fim_db_close(fdb_t *fim_sql);

/**
 * @brief Clean the FIM databases.
 *
 */
void fim_db_clean(void);

/**
 * @brief Compile all statement associated with FIM queries.
 *
 * @param fim_sql FIM database struct.
 *
 * @return FIMDB_OK on success, FIMDB_ERR otherwise.
 */
int fim_db_cache(fdb_t *fim_sql);

/**
 * @brief Finalize all statements.
 *
 * @param fim_sql FIM database struct.
 *
 * @return FIMDB_OK on success, FIMDB_ERR otherwise.
 */
int fim_db_finalize_stmt(fdb_t *fim_sql);

/**
 * @brief End transaction and commit.
 *
 * @param fim_sql FIM database struct.
 */
void fim_db_check_transaction(fdb_t *fim_sql);

/**
 * @brief Force the commit in the database.
 *
 * @param fim_sql FIM database struct.
 */
void fim_db_force_commit(fdb_t *fim_sql);

/**
 * @brief Reset statement and clean bindings parameters.
 *
 * @param fim_sql FIM database struct.
 * @param index Statement index.
 *
 * @return FIMDB_OK on success, FIMDB_ERR otherwise.
 */
int fim_db_clean_stmt(fdb_t *fim_sql, int index);

/**
 * @brief Get count of all entries in the database.
 *
 * The database to count is chosen with the index variable.
 *
 * @param fim_sql FIM database struct.
 * @param index Index to SQL statement.
 *
 * @return Number of entries in selected database.
*/
int fim_db_get_count(fdb_t *fim_sql, int index);

// Callbacks

/**
 * @brief Write an entry path into the storage pointed by @arg.
 *
 * @param fim_sql FIM database struct.
 * @param entry FIM entry to save.
 * @param storage 1 Store database in memory, disk otherwise.
 * @param args Storage which contains all the paths.
 *
 * @return FIMDB_OK on success, FIMDB_ERR otherwise.
 */
void fim_db_callback_save_path(fdb_t *fim_sql, fim_entry *entry, int storage, void *arg);

/**
 * @brief Write a string into the storage pointed by @arg.
 *
 * @param fim_sql FIM database struct.
 * @param str String to be saved into storage.
 * @param storage 1 Store database in memory, disk otherwise.
 * @param arg Storage which contains all the strings.
 *
 * @return FIMDB_OK on success, FIMDB_ERR otherwise.
 */
void fim_db_callback_save_string(fdb_t * fim_sql, char *str, int storage, void *arg);

/**
 * @brief Callback function: Entry checksum calculation.
 *
 * @param fim_sql FIM database struct.
 * @param checksum Checksum to be added to the ongoing digest.
 * @param storage 1 Store database in memory, disk otherwise.
 * @param arg
 */
void fim_db_callback_calculate_checksum(fdb_t *fim_sql, char *checksum, int storage, void *arg);

/**
 * @brief Get checksum of all file_data.
 *
 * @param fim_sql FIM database struct.
 * @param arg CTX object.
 *
 * @return FIMDB_OK on success, FIMDB_ERR otherwise.
 */
int fim_db_get_data_checksum(fdb_t *fim_sql, void * arg);

#endif /* FIM_DB_COMMON_H */
