/*
 * Upload_Download_functional_unit.h
 *
 *  Created on: Jul 9, 2025
 *      Author: suneel.kataraki
 */

#ifndef UPLOAD_DOWNLOAD_FUNCTIONAL_UNIT_H_
#define UPLOAD_DOWNLOAD_FUNCTIONAL_UNIT_H_

#include <stdint.h>
#include <stdbool.h>
#include "uds_services.h"

// Identifiants des services Upload/Download UDS
#define UDS_REQUEST_DOWNLOAD         0x34
#define UDS_REQUEST_UPLOAD           0x35
#define UDS_TRANSFER_DATA            0x36
#define UDS_REQUEST_TRANSFER_EXIT    0x37
#define UDS_REQUEST_FILE_TRANSFER    0x38
#define UDS_RESPONSE_REQUEST_DOWNLOAD    0x74
#define UDS_RESPONSE_TRANSFER_EXIT       0x77



typedef struct {
    uint8_t dataFormatIdentifier;                // Identifiant de format de donn�es
    uint8_t addressAndLengthFormatIdentifier;    // Identifiant de format d'adresse et de longueur
    uint8_t memoryAddress[4];                     // Adresse m�moire (max 4 octets)
    uint8_t memorySize[4];                        // Taille m�moire (max 4 octets)
} RequestDownload_t;

// Structure pour la r�ponse de t�l�chargement
typedef struct {
    uint8_t lengthFormatIdentifier;               // Identifiant de format de longueur
    uint8_t maxNumberOfBlockLength[2];           // Longueur maximale des blocs
} ResponseDownload_t;

// D�finition des Structures
typedef struct {
    uint8_t dataFormatIdentifier; // DFI
    uint8_t addressAndLengthFormatIdentifier; // ALFID
    uint8_t memoryAddress[4]; // Peut �tre ajust� selon la taille
    uint8_t memorySize[4]; // Peut �tre ajust� selon la taille
} RequestUpload_t;

typedef struct {
    uint8_t lengthFormatIdentifier; // LFID
    uint8_t maxNumberOfBlockLength[2]; // Peut �tre ajust� selon la taille
} ResponseUpload_t;

typedef struct {
    uint8_t blockSequenceCounter; // BSC
    uint8_t transferRequestParameterRecord[255]; // Param�tres de requ�te (ajuster selon les besoins)
} RequestTransferData_t;

typedef struct {
    uint8_t blockSequenceCounter; // BSC
    uint8_t transferResponseParameterRecord[255]; // Param�tres de r�ponse (ajuster selon les besoins)
} ResponseTransferData_t;

// Structure pour la requ�te RequestTransferExit
typedef struct {
    uint8_t transferRequestParameterRecord[256]; // Taille arbitraire, ajustez selon vos besoins
} RequestTransferExit_t;

// Structure pour la r�ponse au service
typedef struct {
    uint8_t transferResponseParameterRecord[256]; // Taille arbitraire, ajustez selon vos besoins
} ResponseTransferExit_t;

typedef struct {
    uint8_t modeOfOperation;             // 0x01 - 0x05 : Mode d'op�ration
    uint16_t filePathAndNameLength;      // Longueur du chemin du fichier
    uint8_t filePathAndName[255];        // Chemin du fichier (jusqu'� 255 octets)
    uint8_t dataFormatIdentifier;         // Identifiant du format de donn�es
    uint16_t fileSizeParameterLength;     // Longueur du param�tre de taille de fichier
    uint8_t fileSizeUncompressed[4];      // Taille non compress�e (4 octets)
    uint8_t fileSizeCompressed[4];        // Taille compress�e (4 octets)
} RequestFileTransfer_t;


typedef struct {
    uint8_t SID; // Identifiant de service
    uint8_t modeOfOperation; // �cho du mode d'op�ration
    uint8_t lengthFormatIdentifier; // Longueur du maxNumberOfBlockLength
    uint8_t maxNumberOfBlockLength[2]; // Longueur maximale du bloc
    uint8_t dataFormatIdentifier; // �cho du format de donn�es
    uint16_t fileSizeOrDirInfoParameterLength; // Longueur du param�tre
    uint8_t fileSizeUncompressedOrDirInfoLength[4]; // Taille du fichier
    uint8_t fileSizeCompressed[4]; // Taille compress�e
} ResponseFileTransfer_t;


void uds_request_download(RequestDownload_t *request);
void send_positive_response_request_download(ResponseDownload_t *response);
void send_negative_response_request_download(uint8_t nrc);
bool is_memory_address_valid(uint8_t *address);
bool is_memory_size_valid(uint8_t *size);
bool is_security_active(void);

void uds_request_upload(RequestUpload_t *request);
void send_positive_response_upload(uint8_t *maxNumberOfBlockLength);
void send_negative_response_upload(uint8_t nrc);
bool is_security_active(void);
bool validate_request_upload(RequestUpload_t *request);

void uds_transfer_data(RequestTransferData_t *request);
void send_positive_response_transfer_data(uint8_t blockSequenceCounter);
void send_negative_response_transfer_data(uint8_t nrc);
bool is_request_download_active(void);
bool is_request_upload_active(void);
bool validate_transfer_data_request(RequestTransferData_t *request);

void uds_request_transfer_exit(RequestTransferExit_t *request, ResponseTransferExit_t *response);
void send_positive_response_transfer_exit(ResponseTransferExit_t *response);
void send_negative_response_transfer_exit(uint8_t nrc);
bool is_transfer_in_progress(void);
bool validate_transfer_request_parameters(RequestTransferExit_t *request);

void uds_request_file_transfer(RequestFileTransfer_t *request);
void send_positive_response_file_transfer(ResponseFileTransfer_t *response);
void send_negative_response_file_transfer(uint8_t nrc);

void uds_request_download(RequestDownload_t *request);
void send_positive_response_request_download(ResponseDownload_t *response);
void send_negative_response_request_download(uint8_t nrc);
bool is_memory_address_valid(uint8_t *address);
bool is_memory_size_valid(uint8_t *size);
bool is_security_active(void);

void uds_request_upload(RequestUpload_t *request);
void send_positive_response_upload(uint8_t *maxNumberOfBlockLength);
void send_negative_response_upload(uint8_t nrc);
bool is_security_active(void);
bool validate_request_upload(RequestUpload_t *request);

void uds_transfer_data(RequestTransferData_t *request);
void send_positive_response_transfer_data(uint8_t blockSequenceCounter);
void send_negative_response_transfer_data(uint8_t nrc);
bool is_request_download_active(void);
bool is_request_upload_active(void);
bool validate_transfer_data_request(RequestTransferData_t *request);

void uds_request_transfer_exit(RequestTransferExit_t *request, ResponseTransferExit_t *response);
void send_positive_response_transfer_exit(ResponseTransferExit_t *response);
void send_negative_response_transfer_exit(uint8_t nrc);
bool is_transfer_in_progress(void);
bool validate_transfer_request_parameters(RequestTransferExit_t *request);

void uds_request_file_transfer(RequestFileTransfer_t *request);
void send_positive_response_file_transfer(ResponseFileTransfer_t *response);
void send_negative_response_file_transfer(uint8_t nrc);

#endif /* UPLOAD_DOWNLOAD_FUNCTIONAL_UNIT_H_ */
