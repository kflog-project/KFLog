/***********************************************************************
**
**   MetaTypes.h
**
**   This file is part of KFLog.
**
************************************************************************
**
**   Copyright (c):  2014 by Axel Pauli <kflog.cumulus@gmail.com>
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

/**
 * \author Axel Pauli
 *
 * \brief Different meta type definitions for usage as QVariant.
 *
 *  Different meta type definitions for usage as QVariant.
 *
 *  \date 2014
 *
 *  \version 1.0
 */

#ifndef META_TYPES_H_
#define META_TYPES_H_


#include <QMetaType>

#include "airfield.h"
#include "singlepoint.h"
#include "radiopoint.h"
#include "waypoint.h"

/** Defines an airfield object pointer. */
typedef Airfield* AirfieldPtr;

Q_DECLARE_METATYPE(AirfieldPtr)

/** Defines a radio point object pointer. */
typedef RadioPoint* RadioPointPtr;

Q_DECLARE_METATYPE(RadioPointPtr)

/** Defines a single point object pointer. */
typedef SinglePoint* SinglePointPtr;

Q_DECLARE_METATYPE(SinglePointPtr)

/** Defines a waypoint object pointer. */
typedef Waypoint* WaypointPtr;

Q_DECLARE_METATYPE(WaypointPtr)

#endif /* META_TYPES_H_ */
