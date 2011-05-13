/****************************************************************************/
/// @file    NWWriter_XML.cpp
/// @author  Daniel Krajzewicz
/// @date    Tue, 11.05.2011
/// @version $Id$
///
// Exporter writing networks using XML (native input) format
/****************************************************************************/
// SUMO, Simulation of Urban MObility; see http://sumo.sourceforge.net/
// Copyright (C) 2001-2011 DLR (http://www.dlr.de/) and contributors
/****************************************************************************/
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//
/****************************************************************************/


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif
#include "NWWriter_XML.h"
#include <utils/common/MsgHandler.h>
#include <netbuild/NBEdge.h>
#include <netbuild/NBEdgeCont.h>
#include <netbuild/NBNode.h>
#include <netbuild/NBNodeCont.h>
#include <netbuild/NBNetBuilder.h>
#include <utils/common/ToString.h>
#include <utils/options/OptionsCont.h>
#include <utils/iodevices/OutputDevice.h>
#include <utils/geom/GeoConvHelper.h>

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS



// ===========================================================================
// method definitions
// ===========================================================================
// ---------------------------------------------------------------------------
// static methods
// ---------------------------------------------------------------------------
void
NWWriter_XML::writeNetwork(const OptionsCont &oc, NBNetBuilder &nb) {
    // check whether a matsim-file shall be generated
    if (!oc.isSet("plain-output-prefix")) {
        return;
    }
    // write nodes
	OutputDevice& device = OutputDevice::getDevice(oc.getString("plain-output-prefix") + ".nod.xml");
    device.writeXMLHeader("nodes");
	NBNodeCont &nc = nb.getNodeCont();
	for(std::map<std::string, NBNode*>::const_iterator i=nc.begin(); i!=nc.end(); ++i) {
        NBNode *n = (*i).second;
        device << "   <node id=\"" << n->getID() << "\" ";
        if (GeoConvHelper::usingInverseGeoProjection()) {
            device.setPrecision(GEO_OUTPUT_ACCURACY);
            device << "x=\"" << n->getPosition().x() << "\" y=\"" << n->getPosition().y() << "\"";
            device.setPrecision();
        } else {
            device << "x=\"" << n->getPosition().x() << "\" y=\"" << n->getPosition().y() << "\"";
        }
        device << " type=\"" << toString(n->getType())<< "\"";
        if (n->isTLControlled()) {
            device << " tl=\"";
            const std::set<NBTrafficLightDefinition*> &tlss = n->getControllingTLS();
            for (std::set<NBTrafficLightDefinition*>::const_iterator t=tlss.begin(); t!=tlss.end(); ++t) {
                if (t!=tlss.begin()) {
                    device << ",";
                }
                device << (*t)->getID();
            }
            device << "\"";
        }
        device << "/>\n";
    }
    device.close();
    // write edges / connections
    OutputDevice& edevice = OutputDevice::getDevice(oc.getString("plain-output-prefix") + ".edg.xml");
    edevice.writeXMLHeader("edges");
    OutputDevice& cdevice = OutputDevice::getDevice(oc.getString("plain-output-prefix") + ".con.xml");
    cdevice.writeXMLHeader("connections");
	NBEdgeCont &ec = nb.getEdgeCont();
	for(std::map<std::string, NBEdge*>::const_iterator i=ec.begin(); i!=ec.end(); ++i) {
        // write the edge itself to the edges-files
        NBEdge *e = (*i).second;
        edevice << "   <edge id=\"" << e->getID()
        << "\" fromnode=\"" << e->getFromNode()->getID()
        << "\" tonode=\"" << e->getToNode()->getID()
        << "\" priority=\"" << e->getPriority();
        // write the type if given
        if (e->getTypeID() != "") {
            edevice << "\" type=\"" << e->getTypeID();
        }
        edevice << "\" nolanes=\"" << e->getNoLanes()
        << "\" speed=\"" << e->getSpeed() << "\"";
        // write the geometry only if larger than just the from/to positions
        if (e->getGeometry().size() > 2) {
            edevice << " shape=\"" << e->getGeometry() << "\"";
        }
        // write the spread type if not default ("right")
        if (e->getLaneSpreadFunction()!=LANESPREAD_RIGHT) {
            edevice << " spread_type=\"" << toString(e->getLaneSpreadFunction()) << "\"";
        }
        // write the vehicles class if restrictions exist
        if (!e->hasRestrictions()) {
            edevice << "/>\n";
        } else {
            edevice << ">\n";
            e->writeLanesPlain(edevice);
            edevice << "   </edge>\n";
        }
        // write this edge's connections to the connections-files
        unsigned int noLanes = e->getNoLanes();
        unsigned int noWritten = 0;
        for (unsigned int lane=0; lane<noLanes; ++lane) {
            std::vector<NBEdge::Connection> connections = e->getConnectionsFromLane(lane);
            for (std::vector<NBEdge::Connection>::iterator c=connections.begin(); c!=connections.end(); ++c) {
                if ((*c).toEdge!=0) {
                    cdevice << "	<connection from=\"" << e->getID()
                    << "\" to=\"" << (*c).toEdge->getID()
                    << "\" lane=\"" << (*c).fromLane << ":" << (*c).toLane;
                    cdevice << "\"/>\n";
                    ++noWritten;
                }
            }
        }
        if (noWritten>0) {
            cdevice << "\n";
        }
    }
    edevice.close();
    cdevice.close();
}


/****************************************************************************/
