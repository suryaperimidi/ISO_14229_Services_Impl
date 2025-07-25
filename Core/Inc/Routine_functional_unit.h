/*
 * Routine_functional_unit.h
 *
 *  Created on: Jul 9, 2025
 *      Author: suneel.kataraki
 */

#ifndef ROUTINE_FUNCTIONAL_UNIT_C_
#define ROUTINE_FUNCTIONAL_UNIT_C_
#include "uds_services.h"

#include <stdint.h>
#include <stdbool.h>


// Identifiant du service RoutineControl
#define UDS_ROUTINE_CONTROL 0x31
#define UDS_RESPONSE_ROUTINE_CONTROL     0x71

// Sous-fonctions pour RoutineControl
#define ROUTINE_CONTROL_START 0x01
#define ROUTINE_CONTROL_STOP 0x02
#define ROUTINE_CONTROL_REQUEST_RESULTS 0x03



// Structure pour la requ�te RoutineControl
// Structure pour la requ�te RoutineControl
typedef struct {
    uint8_t subFunction; // Sous-fonction (d�marrer, arr�ter, demander des r�sultats)
    uint16_t routineIdentifier; // Identifiant de la routine (2 octets)
    uint8_t routineControlOption; // Param�tres optionnels pour la routine
} RoutineControlRequest_t;

// Structure pour la r�ponse RoutineControl
typedef struct {
    uint8_t SID;                // Identifiant de service de r�ponse
    uint8_t routineControlType; // Type de contr�le de routine
    uint16_t routineIdentifier;  // Identifiant de la routine
    uint8_t routineInfo;        // Informations sur la routine
    uint8_t routineStatusRecord[8]; // Statut de la routine
} RoutineControlResponse_t;

void uds_routine_control(RoutineControlRequest_t *request, RoutineControlResponse_t *response);
void send_positive_response_routine_control(RoutineControlResponse_t *response);
void send_negative_response_routine_control(uint8_t nrc);
bool is_routine_supported(uint16_t routineIdentifier);
bool is_security_granted_for_routine(uint16_t routineIdentifier);
bool is_sub_function_supported(uint8_t subFunction);
bool validate_routine_control_option(uint8_t option);



#endif /* ROUTINE_FUNCTIONAL_UNIT_C_ */
