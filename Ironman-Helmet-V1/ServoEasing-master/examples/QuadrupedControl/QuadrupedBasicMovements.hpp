/*
 * QuadrupedBasicMovements.hpp
 *
 * This file  contains the basic movement functions
 *
 *  Copyright (C) 2019-2022  Armin Joachimsmeyer
 *  armin.joachimsmeyer@gmail.com
 *
 *  This file is part of QuadrupedControl https://github.com/ArminJo/QuadrupedControl.
 *
 *  QuadrupedControl is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/gpl.html>.
 */

#ifndef _QUADRUPED_BASIC_MOVEMENTS_HPP
#define _QUADRUPED_BASIC_MOVEMENTS_HPP

#include <Arduino.h>

#include "QuadrupedBasicMovements.h"
#include "QuadrupedServoControl.h"

//#define INFO // activate this to see serial info output

volatile uint8_t sMovingDirection = MOVE_DIRECTION_FORWARD;

uint8_t sCurrentlyRunningAction; // A change on this action type triggers the generation of new NeoPatterns
uint8_t sLastActionTypeForNeopatternsDisplay; // do determine changes of sCurrentlyRunningAction

/*
 * Gait variations
 * 1. Creep: Move one leg forward and down, then move body with all 4 legs down, then move diagonal leg.
 * 2. Trot: Move 2 diagonal legs up and forward
 */

void moveTrot(uint8_t aNumberOfTrots) {
    sCurrentlyRunningAction = ACTION_TYPE_TROT;
    setEasingTypeForMoving();

    do {
        uint8_t tCurrentDirection = sMovingDirection;
        uint8_t tRequestedBodyHeightAngle = sRequestedBodyHeightAngle; // sRequestedBodyHeightAngle is volatile
        uint8_t LiftMaxAngle = tRequestedBodyHeightAngle + ((LIFT_HIGHEST_ANGLE - tRequestedBodyHeightAngle) / 2);
        /*
         * first move right front and left back leg up and forward
         */
        transformAndSetAllServos(TROT_BASE_ANGLE_FL_BR + TROT_MOVE_ANGLE, TROT_BASE_ANGLE_BL_FR - TROT_MOVE_ANGLE,
        TROT_BASE_ANGLE_FL_BR - TROT_MOVE_ANGLE, TROT_BASE_ANGLE_BL_FR + TROT_MOVE_ANGLE, tRequestedBodyHeightAngle, LiftMaxAngle,
                tRequestedBodyHeightAngle, LiftMaxAngle, tCurrentDirection);
        QUADRUPED_BREAK_IF_STOP;

        tRequestedBodyHeightAngle = sRequestedBodyHeightAngle;

        // and the the other legs
        transformAndSetAllServos(TROT_BASE_ANGLE_FL_BR - TROT_MOVE_ANGLE, TROT_BASE_ANGLE_BL_FR + TROT_MOVE_ANGLE,
        TROT_BASE_ANGLE_FL_BR + TROT_MOVE_ANGLE, TROT_BASE_ANGLE_BL_FR - TROT_MOVE_ANGLE, LiftMaxAngle, tRequestedBodyHeightAngle,
                LiftMaxAngle, tRequestedBodyHeightAngle, tCurrentDirection);
        QUADRUPED_BREAK_IF_STOP;

        aNumberOfTrots--;
    } while (aNumberOfTrots != 0);

    setEasingTypeToLinear();
    sCurrentlyRunningAction = ACTION_TYPE_STOP;
}

void basicTwist(int8_t aTwistAngle, bool aTurnLeft) {
    sCurrentlyRunningAction = ACTION_TYPE_TWIST;
#if defined(INFO)
    Serial.print(F("Twist angle="));
    Serial.print(aTwistAngle);
    if (aTurnLeft) {
        Serial.println(F(" left"));
    } else {
        Serial.println(F(" right"));
    }
#endif
    int8_t tTwistAngle;
    aTurnLeft ? tTwistAngle = -aTwistAngle : tTwistAngle = aTwistAngle;

    setPivotServos(90 + tTwistAngle, 90 + tTwistAngle, 90 + tTwistAngle, 90 + tTwistAngle);
    sCurrentlyRunningAction = ACTION_TYPE_STOP;
}

/*
 * Must reverse direction of legs if turning right otherwise the COG (Center Of Gravity) is not supported.
 */
void moveTurn(uint8_t aNumberOfBasicTurns) {
    sCurrentlyRunningAction = ACTION_TYPE_TURN;
    centerServos();
    setEasingTypeForMoving();
    uint8_t tNextLiftLegIndex = FRONT_LEFT;

    /*
     * First move one leg out of center position, otherwise the COG (Center Of Gravity) is not supported at the first move
     */
    if (sMovingDirection == MOVE_DIRECTION_LEFT) {
        moveOneServoAndCheckInputAndWait(FRONT_RIGHT_PIVOT, TURN_LEFT_START_ANGLE);
    } else {
        moveOneServoAndCheckInputAndWait(BACK_LEFT_PIVOT, TURN_LEFT_END_ANGLE);
    }

    // do at least one basic turn
    do {
        QUADRUPED_BREAK_IF_STOP;
        basicTurn(tNextLiftLegIndex, sMovingDirection == MOVE_DIRECTION_LEFT);
        /*
         * get next index of leg which must be lifted
         */
        sMovingDirection == MOVE_DIRECTION_LEFT ? tNextLiftLegIndex++ : tNextLiftLegIndex--; // Reverse the direction of NextLegIndex if moveTurn right
        tNextLiftLegIndex = tNextLiftLegIndex % NUMBER_OF_LEGS;

    }while (--aNumberOfBasicTurns != 0);

    setEasingTypeToLinear();
    sCurrentlyRunningAction = ACTION_TYPE_STOP;
}

/*
 * One basicTurn() call turns the body by approximately 12 degree
 * The aLiftLegIndex is lifted up and moved by 120 degree in body turn direction
 * All other legs are moved by 40 degree against the body turn direction
 *
 * @param aLiftLegIndex from 0 FRONT_LEFT to 3 FRONT_RIGHT
 */
void basicTurn(uint8_t aLiftLegIndex, bool aTurnLeft) {
#if defined(INFO)
    Serial.print(F("Lift leg="));
    Serial.print(aLiftLegIndex);
    if (aTurnLeft) {
        Serial.println(F(" left"));
    } else {
        Serial.println(F(" right"));
    }
#endif
    int_fast8_t tLiftLegServoIndex = aLiftLegIndex * SERVOS_PER_LEG;
    int8_t tForwardAngle;

    /*
     * Lift and move one leg forward in sMovingDirection direction
     */
    if (aTurnLeft) {
        ServoEasing::ServoEasingNextPositionArray[tLiftLegServoIndex] = TURN_LEFT_START_ANGLE;
        tForwardAngle = -TURN_LEFT_STEP_ANGLE;
    } else {
        ServoEasing::ServoEasingNextPositionArray[tLiftLegServoIndex] = TURN_LEFT_END_ANGLE;
        tForwardAngle = TURN_LEFT_STEP_ANGLE;
    }
    // write desired position to array for later apply
    ServoEasing::ServoEasingNextPositionArray[tLiftLegServoIndex + LIFT_SERVO_OFFSET] = LIFT_HIGHEST_ANGLE;

    int_fast8_t tTurnLegServoIndex = tLiftLegServoIndex + SERVOS_PER_LEG; // get servo index of next leg
    // write new positions for the other 3 pivot servos
    uint8_t tRequestedBodyHeightAngle = sRequestedBodyHeightAngle; // sRequestedBodyHeightAngle is volatile (and code is smaller)
    for (uint_fast8_t i = 0; i < NUMBER_OF_LEGS - 1; ++i) {
        tTurnLegServoIndex %= NUMBER_OF_SERVOS;
        ServoEasing::ServoEasingNextPositionArray[tTurnLegServoIndex] =
                ServoEasing::ServoEasingNextPositionArray[tTurnLegServoIndex] + tForwardAngle;
        // (re)set lift value for this leg
        ServoEasing::ServoEasingNextPositionArray[tTurnLegServoIndex + LIFT_SERVO_OFFSET] = tRequestedBodyHeightAngle;
        tLiftLegServoIndex += SERVOS_PER_LEG;  // get servo of next leg
    }
//    printArrayPositions(&Serial);
    /*
     * Apply all movements now
     */
    synchronizeMoveAllServosAndCheckInputAndWait();
}

/*
 * Y position with right legs closed and left legs open
 */
void goToYPosition(uint8_t aDirection) {
#if defined(INFO)
    Serial.print(F("goToYPosition aDirection="));
    Serial.println(aDirection);
#endif
    transformAndSetPivotServos(180 - Y_POSITION_OPEN_ANGLE, Y_POSITION_OPEN_ANGLE, (180 - Y_POSITION_CLOSE_ANGLE),
    Y_POSITION_CLOSE_ANGLE, aDirection, false, false);
    setLiftServos(sRequestedBodyHeightAngle);
}

/*
 * 0 -> 256 creeps
 */
void moveCreep(uint8_t aNumberOfCreeps) {
    sCurrentlyRunningAction = ACTION_TYPE_CREEP;
    goToYPosition(sMovingDirection);
    setEasingTypeForMoving();

    do {
        // read current direction for next 2  creeps
        uint8_t tCurrentDirection = sMovingDirection;

        basicHalfCreep(tCurrentDirection, false);
        QUADRUPED_BREAK_IF_STOP;
// now mirror movement
        basicHalfCreep(tCurrentDirection, true);
        QUADRUPED_BREAK_IF_STOP;
        aNumberOfCreeps--;
    } while (aNumberOfCreeps != 0);

    setEasingTypeToLinear();
    sCurrentlyRunningAction = ACTION_TYPE_STOP;
#if defined(INFO)
    Serial.println(F("Creep ended"));
#endif
}

/*
 * moves one leg forward and down, then moves body, then moves diagonal leg.
 */
void basicHalfCreep(uint8_t aDirection, bool doMirror) {
#if defined(INFO)
    Serial.print(F("BasicHalfCreep Direction="));
    Serial.print(aDirection);
    Serial.print(F(" doMirror="));
    Serial.println(doMirror);
// 1. Move front right leg up, forward and down
    Serial.println(F("1. move front leg"));
#endif
    uint8_t tRequestedBodyHeightAngle = sRequestedBodyHeightAngle; // sRequestedBodyHeightAngle is volatile
    transformAndSetAllServos(180 - Y_POSITION_OPEN_ANGLE, Y_POSITION_OPEN_ANGLE, 180 - Y_POSITION_CLOSE_ANGLE,
    Y_POSITION_FRONT_ANGLE, tRequestedBodyHeightAngle, tRequestedBodyHeightAngle, tRequestedBodyHeightAngle, LIFT_HIGHEST_ANGLE,
            aDirection, doMirror);
    QUADRUPED_RETURN_IF_STOP;

    tRequestedBodyHeightAngle = sRequestedBodyHeightAngle;

// 2. Move body forward by CREEP_BODY_MOVE_ANGLE
#if defined(INFO)
    Serial.println(F("2. move body"));
#endif
    transformAndSetAllServos(180 - Y_POSITION_CLOSE_ANGLE, Y_POSITION_OPEN_ANGLE + CREEP_BODY_MOVE_ANGLE,
            180 - Y_POSITION_OPEN_ANGLE, Y_POSITION_OPEN_ANGLE, tRequestedBodyHeightAngle, tRequestedBodyHeightAngle,
            tRequestedBodyHeightAngle, tRequestedBodyHeightAngle, aDirection, doMirror);
    QUADRUPED_RETURN_IF_STOP;

    tRequestedBodyHeightAngle = sRequestedBodyHeightAngle;

// 3. Move back right leg up, forward and down
#if defined(INFO)
    Serial.println(F("3. move back leg to close position"));
#endif
// Move to Y position with other side legs together
    transformAndSetAllServos(180 - Y_POSITION_CLOSE_ANGLE, Y_POSITION_CLOSE_ANGLE, 180 - Y_POSITION_OPEN_ANGLE,
    Y_POSITION_OPEN_ANGLE, tRequestedBodyHeightAngle, LIFT_HIGHEST_ANGLE, tRequestedBodyHeightAngle, tRequestedBodyHeightAngle,
            aDirection, doMirror);
}

/*
 * Just as an unused example to see the principle of movement
 */
void basicSimpleHalfCreep(uint8_t aLeftLegIndex, bool aMoveMirrored) {
#if defined(INFO)
    Serial.print(F("LeftLegIndex="));
    Serial.print(aLeftLegIndex);
#endif

    uint8_t tLeftLegPivotServoIndex;

    if (aMoveMirrored) {
#if defined(INFO)
        Serial.print(F(" mirrored=true"));
#endif
// get index of pivot servo of mirrored leg
        tLeftLegPivotServoIndex = ((NUMBER_OF_LEGS - 1) - aLeftLegIndex) * SERVOS_PER_LEG; // 0->6
    } else {
        tLeftLegPivotServoIndex = aLeftLegIndex * SERVOS_PER_LEG;
    }
#if defined(INFO)
    Serial.println();
#endif
//    printArrayPositions(&Serial);
    uint8_t tEffectiveAngle;

// 1. Move front left leg up, forward and down
#if defined(INFO)
    Serial.println(F("1. move front leg"));
#endif
    moveOneServoAndCheckInputAndWait(tLeftLegPivotServoIndex + LIFT_SERVO_OFFSET, LIFT_HIGHEST_ANGLE);
    QUADRUPED_RETURN_IF_STOP;

// go CREEP_BODY_MOVE_ANGLE ahead of Y_POSITION_OPEN_ANGLE
    aMoveMirrored ?
            tEffectiveAngle = 180 - (Y_POSITION_OPEN_ANGLE - CREEP_BODY_MOVE_ANGLE) :
            tEffectiveAngle = Y_POSITION_OPEN_ANGLE - CREEP_BODY_MOVE_ANGLE;
    moveOneServoAndCheckInputAndWait(tLeftLegPivotServoIndex, tEffectiveAngle);
    QUADRUPED_RETURN_IF_STOP;

    moveOneServoAndCheckInputAndWait(tLeftLegPivotServoIndex + LIFT_SERVO_OFFSET, sRequestedBodyHeightAngle);
    QUADRUPED_RETURN_IF_STOP;

// 2. Move body forward
#if defined(INFO)
    Serial.println(F("2. move body"));
#endif
    uint8_t tIndex = tLeftLegPivotServoIndex;
    uint8_t tIndexDelta;
// Front left
    if (aMoveMirrored) {
        ServoEasing::ServoEasingNextPositionArray[tIndex] = 180 - Y_POSITION_OPEN_ANGLE;
        tIndexDelta = -SERVOS_PER_LEG;
    } else {
        ServoEasing::ServoEasingNextPositionArray[tIndex] = Y_POSITION_OPEN_ANGLE;
        tIndexDelta = SERVOS_PER_LEG;
    }
// Back left
    tIndex = (tIndex + tIndexDelta) % NUMBER_OF_SERVOS;
    if (aMoveMirrored) {
        ServoEasing::ServoEasingNextPositionArray[tIndex] = Y_POSITION_OPEN_ANGLE;
    } else {
        ServoEasing::ServoEasingNextPositionArray[tIndex] = 180 - Y_POSITION_OPEN_ANGLE;
    }

// Back right
    tIndex = (tIndex + tIndexDelta) % NUMBER_OF_SERVOS;
    if (aMoveMirrored) {
        ServoEasing::ServoEasingNextPositionArray[tIndex] = 180 - CREEP_BODY_MOVE_ANGLE;
    } else {
        ServoEasing::ServoEasingNextPositionArray[tIndex] = CREEP_BODY_MOVE_ANGLE;
    }

// Front right
    tIndex = (tIndex + tIndexDelta) % NUMBER_OF_SERVOS;
    if (aMoveMirrored) {
        ServoEasing::ServoEasingNextPositionArray[tIndex] = Y_POSITION_CLOSE_ANGLE;
    } else {
        ServoEasing::ServoEasingNextPositionArray[tIndex] = 180 - Y_POSITION_CLOSE_ANGLE;
    }
//    printArrayPositions(&Serial);
    synchronizeMoveAllServosAndCheckInputAndWait();

// 3. Move back right leg up, forward and down
#if defined(INFO)
    Serial.println(F("3. move back leg to close position"));
#endif
// Move to Y position with right legs together / 120, 60, 180, 0
    uint8_t tDiagonalIndex = (tLeftLegPivotServoIndex + DIAGONAL_SERVO_OFFSET) % NUMBER_OF_SERVOS;
    moveOneServoAndCheckInputAndWait(tDiagonalIndex + LIFT_SERVO_OFFSET, LIFT_HIGHEST_ANGLE);
    QUADRUPED_RETURN_IF_STOP;

    aMoveMirrored ? tEffectiveAngle = 180 - Y_POSITION_CLOSE_ANGLE : tEffectiveAngle = Y_POSITION_CLOSE_ANGLE;
    moveOneServoAndCheckInputAndWait(tDiagonalIndex, tEffectiveAngle);
    QUADRUPED_RETURN_IF_STOP;

    moveOneServoAndCheckInputAndWait(tDiagonalIndex + LIFT_SERVO_OFFSET, sRequestedBodyHeightAngle);
}
#endif // _QUADRUPED_BASIC_MOVEMENTS_HPP
