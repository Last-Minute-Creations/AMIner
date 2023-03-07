/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef AMINER_FACE_ID_H
#define AMINER_FACE_ID_H

/**
 * @brief Face id.
 * Must be same order as in tMsg enum!
 */
typedef enum _tFaceId {
	FACE_ID_MIETEK,
	FACE_ID_KRYSTYNA,
	FACE_ID_KOMISARZ,
	FACE_ID_URZEDAS,
	FACE_ID_ARCH,
	FACE_ID_PRISONER,
	FACE_ID_AGENT,
	FACE_ID_COUNT,
} tFaceId;

#endif // AMINER_FACE_ID_H
