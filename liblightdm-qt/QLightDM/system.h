/*
 * Copyright (C) 2010-2011 David Edmundson.
 * Copyright (C) 2010-2011 Robert Ancell
 * Author: David Edmundson <kde@davidedmundson.co.uk>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 3 of the License, or (at your option) any
 * later version. See http://www.gnu.org/copyleft/lgpl.html the full text of the
 * license.
 */

#ifndef QLIGHTDM_SYSTEM_H
#define QLIGHTDM_SYSTEM_H

#include <QtCore/QString>

namespace QLightDM
{
    class System
    {
    public:
        static QString hostname();
    };
};

#endif // QLIGHTDM_SYSTEM_H
