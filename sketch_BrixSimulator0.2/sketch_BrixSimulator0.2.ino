/* Edge Impulse ingestion SDK
 * Copyright (c) 2022 EdgeImpulse Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

/* Includes ---------------------------------------------------------------- */
#include <BrixSimulator_inferencing.h>
#include "DFRobot_AS7341.h" //Sensor
#include <Wire.h>           //Comunicación I2C con RTC

DFRobot_AS7341 as7341;
int buttonPin = D7;           //Pin donde se conecta el botón
#define SerialLogger Serial2  //Usamos Serial2 para el Data Logger

float features[10];

//BRix entre 17.4 y 24.6
float filas0[][10] = {
  {87, 600, 223, 467, 705, 636, 485, 231, 1000, 82},    // Fila 802
  {68, 466, 176, 379, 484, 455, 360, 167, 1000, 64},    // Fila 803
  {60, 400, 153, 331, 511, 463, 368, 182, 1000, 67},    // Fila 804
  {64, 418, 160, 354, 546, 494, 392, 187, 1000, 70},    // Fila 805
  {81, 537, 199, 434, 650, 596, 458, 214, 1000, 78},    // Fila 806
  {109, 763, 280, 595, 915, 814, 624, 302, 1000, 102},  // Fila 807
  {65, 437, 162, 352, 529, 477, 383, 187, 1000, 70},    // Fila 808
  {63, 422, 163, 341, 521, 486, 378, 183, 1000, 68},    // Fila 809
  {71, 452, 173, 381, 585, 538, 420, 199, 1000, 70},    // Fila 810
  {68, 459, 175, 370, 569, 519, 409, 197, 1000, 70},    // Fila 811
};
//Brix entre 5.0 y 5.7
float filas1[][10] = {
  {103, 503, 232, 722, 1000, 1000, 790, 476, 1000, 139},    // Fila 72
  {114, 552, 260, 882, 1000, 1000, 869, 447, 1000, 152},    // Fila 73
  {132, 683, 302, 980, 1000, 1000, 920, 492, 1000, 172},    // Fila 74
  {133, 691, 307, 979, 1000, 1000, 945, 509, 1000, 165},    // Fila 75
  {99, 492, 225, 720, 1000, 1000, 734, 415, 1000, 128},     // Fila 76
  {125, 632, 297, 937, 1000, 1000, 950, 495, 1000, 163},    // Fila 77
  {114, 558, 259, 866, 1000, 1000, 841, 455, 1000, 150},    // Fila 78
  {139, 709, 324, 1000, 1000, 1000, 1000, 547, 1000, 175},  // Fila 79
  {114, 572, 259, 834, 1000, 1000, 847 ,460, 1000, 148},    // Fila 80
  {127, 635, 284, 928, 1000, 1000, 926, 497, 1000, 165},    // Fila 81
};
//Brix entre 8.8 y 12.5
float filas2[][10] = {
  {95, 599, 229, 479, 740, 797, 737, 400, 1000, 104},   // Fila 262
  {110, 714, 280, 570, 931, 980, 846, 483, 1000, 121},  // Fila 263
  {118, 806, 301, 609, 930, 949, 848, 456, 1000, 120},  // Fila 264
  {85, 544, 210, 443, 684, 715, 668, 371, 1000, 96},    // Fila 265
  {87, 578, 220, 466, 715, 699, 657, 385, 1000, 92},    // Fila 266
  {104, 683, 266, 531, 841, 880, 788, 457, 1000, 114},  // Fila 267
  {107, 670, 263, 556, 880, 951, 830, 449, 1000, 120},  // Fila 268
  {73, 442, 180, 368, 561, 591, 537, 308, 1000, 84},    // Fila 269
  {98, 673, 257, 496, 762, 761, 708, 418, 1000, 106},   // Fila 270
  {93, 606, 228, 467, 713, 739, 694, 383, 1000, 100},   // Fila 271
};
//352-361 (entre 7.1 y 9.9)

/**
 * @brief      Copy raw feature data in out_ptr
 *             Function called by inference library
 *
 * @param[in]  offset   The offset
 * @param[in]  length   The length
 * @param      out_ptr  The out pointer
 *
 * @return     0
 */
int raw_feature_get_data(size_t offset, size_t length, float *out_ptr) {
    memcpy(out_ptr, features + offset, length * sizeof(float));
    return 0;
}

void print_inference_result(ei_impulse_result_t result);

/**
 * @brief      Arduino setup function
 */
void setup()
{
    // put your setup code here, to run once:
    Serial.begin(115200);
    // comment out the below line to cancel the wait for USB connection (needed for native USB)
    while (!Serial);
    Serial.println("Edge Impulse Inferencing Demo");

    SerialLogger.begin(9600, SERIAL_8N1, 17, 16);  //Conexión para el Data Logger con pines GPIO adecuados

    //Detectar si IIC se comunica correctamente
    while (as7341.begin() != 0) {
        Serial.println("Fallo iniciando IIC, por favor comprueba si la conexión es correcta.");
        delay(1000);
    }

    pinMode(buttonPin, INPUT);     //Declarara botón como input
}

/**
 * @brief      Arduino main function
 */
void loop()
{
    if (digitalRead(buttonPin) == HIGH) {
        for(int i=0; i<10; i++){
            memcpy(features, filas2[i], sizeof(filas2[i]));

            ei_impulse_result_t result = { 0 };

            // the features are stored into flash, and we don't want to load everything into RAM
            signal_t features_signal;
            features_signal.total_length = sizeof(features) / sizeof(features[0]);
            features_signal.get_data = &raw_feature_get_data;

            // invoke the impulse
            EI_IMPULSE_ERROR res = run_classifier(&features_signal, &result, false /* debug */);
            if (res != EI_IMPULSE_OK) {
                ei_printf("ERR: Failed to run classifier (%d)\n", res);
                return;
            }

            // print inference return code
            Serial.print("Fila: ");
            Serial.println(262+i);
            print_inference_result(result);
        } 
    }
    delay(1000);
}

void print_inference_result(ei_impulse_result_t result) {
    ei_printf("Predictions:\r\n");
    for (uint16_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
        ei_printf("  %s: ", ei_classifier_inferencing_categories[i]);
        ei_printf("%.5f\r\n", result.classification[i].value);
    }
}