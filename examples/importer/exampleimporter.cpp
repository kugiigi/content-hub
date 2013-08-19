/*
 * Copyright (C) 2013 Canonical, Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Ken VanDine <ken.vandine@canonical.com>
 */

#include "exampleimporter.h"


void ExampleImporter::handle_export(cuc::Transfer *transfer)
{
    qDebug() << "handle_export not implemented";
    transfer->state();
}

void ExampleImporter::handle_import(cuc::Transfer *transfer)
{
    qDebug() << "handle_import for:" << transfer->collect().count() << "items";
}
