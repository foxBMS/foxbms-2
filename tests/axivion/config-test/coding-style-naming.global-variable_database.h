// @prefix  DATA

#ifndef FOXBMS__CODING_STYLE_NAMING_GLOBAL_VARIABLE_DATABASE_H_
#define FOXBMS__CODING_STYLE_NAMING_GLOBAL_VARIABLE_DATABASE_H_

/* see src/app/engine/config/database_cfg.h for an actual implementation of
 * - DATA_BLOCK_ID_e
 * - DATA_BLOCK_HEADER_s
 * - DATA_BLOCK_SOC_s
 */
typedef enum {
    DATA_BLOCK_ID_SOC,
    DATA_BLOCK_ID_MAX, /**< DO NOT CHANGE, MUST BE THE LAST ENTRY */
} DATA_BLOCK_ID_e;

typedef struct {
    DATA_BLOCK_ID_e uniqueId; /*!< uniqueId of database entry */
} DATA_BLOCK_HEADER_s;

typedef struct {
    DATA_BLOCK_HEADER_s header; /*!< Data block header */
    int soc;
} DATA_BLOCK_SOC_s;

#endif /* FOXBMS__CODING_STYLE_NAMING_GLOBAL_VARIABLE_DATABASE_H_ */
