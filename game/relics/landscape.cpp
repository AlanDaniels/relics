
#include "stdafx.h"
#include "landscape.h"
#include "chunk.h"


// Our only allowed constructor.
Landscape::Landscape(Chunk &owner) :
    m_owner(owner)
{

}


// Destructor.
Landscape::~Landscape()
{
    freeSurfaceLists();
}


// Return a surface vert list, read-only.
// Note that this can return a null.
const VertList_PNT *Landscape::getSurfaceList_RO(SurfaceType surf) const
{
    int index = static_cast<int>(surf);
    return m_vert_lists.at(index).get();
}


// Return a surface vert list, for writing.
// If it doesn't exist, create it.
VertList_PNT &Landscape::getSurfaceList_RW(SurfaceType surf)
{
    int index = static_cast<int>(surf);
    if (m_vert_lists.at(index) == nullptr) {
        int i = static_cast<int>(surf);
        m_vert_lists[i] = std::make_unique<VertList_PNT>();
    }
    return *m_vert_lists.at(index);
}


// Return the count for a particular surface type.
int Landscape::getCountForSurface(SurfaceType surf) const
{
    int index = static_cast<int>(surf);
    if (m_vert_lists.at(index) == nullptr) {
        return 0;
    }
    else {
        int result = m_vert_lists.at(index)->getByteCount();
        return result;
    }
}


// Rebuild our surface lists.
void Landscape::rebuildSurfaceLists()
{
    // Recalc our exposures, both inner and along edges.
    SurfaceTotals totals;

    // Clear out our surface lists.
    for (int i = 0; i < SURFACE_TYPE_COUNT; i++) {
        if (m_vert_lists.at(i) != nullptr) {
            m_vert_lists.at(i)->reset();
        }
    }

    // Populate those surface lists.
    std::vector<LocalGrid> coords = m_owner.getExposedBlockList();

    for (LocalGrid coord : coords) {
        m_owner.addToSurfaceLists(coord);
    }

    // All done.
    for (int i = 0; i < SURFACE_TYPE_COUNT; i++) {
        if (m_vert_lists.at(i) != nullptr) {
            m_vert_lists.at(i)->update();
        }
    }

    const auto &origin = m_owner.getOrigin();

    const int grand_total = totals.getGrandTotal();
    if (grand_total == 0) {
        PrintDebug(fmt::format(
            "Recalculated all exposures for [{0}, {1}].\n",
            origin.debugX(), origin.debugZ()));
    }
    else {
        PrintDebug(fmt::format(
            "Recalculated all exposures for [{0}, {1}]. Found {2} surfaces total.\n",
            origin.debugX(), origin.debugZ(), grand_total));
    }
}


// Free up any surface lists.
void Landscape::freeSurfaceLists() {
    for (int i = 0; i < SURFACE_TYPE_COUNT; i++) {
        if (m_vert_lists.at(i) != nullptr) {
            m_vert_lists.at(i)->reset();
        }
    }

    m_vert_lists.empty();
}