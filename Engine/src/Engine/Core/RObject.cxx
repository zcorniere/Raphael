#include "Engine/Core/RObject.hxx"

#include <unordered_set>

static std::unordered_set<RObject*> s_LiveReferences;
static std::recursive_mutex s_LiveReferenceMutex;

void RObjectUtils::AddToLiveReferences(RObject* instance)
{
    check(instance);

    std::scoped_lock lock(s_LiveReferenceMutex);
    s_LiveReferences.insert(instance);
}

void RObjectUtils::RemoveFromLiveReferences(RObject* instance)
{
    check(instance);

    std::scoped_lock lock(s_LiveReferenceMutex);
    check(s_LiveReferences.find(instance) != s_LiveReferences.end());
    s_LiveReferences.erase(instance);
}

bool RObjectUtils::IsLive(RObject* instance)
{
    check(instance);
    std::scoped_lock lock(s_LiveReferenceMutex);
    return s_LiveReferences.find(instance) != s_LiveReferences.end();
}

bool RObjectUtils::AreThereAnyLiveObject(bool bPrintObjects)
{
    std::scoped_lock lock(s_LiveReferenceMutex);

    if (bPrintObjects) {
        for (RObject* ObjectPtr: s_LiveReferences) {
            LOG(LogRObject, Trace, "{}<{}> ({:p}) have {} references", ObjectPtr->GetBaseTypeName(),
                ObjectPtr->GetName(), (void*)ObjectPtr, ObjectPtr->GetRefCount());
        }
    }
    return s_LiveReferences.size() > 0;
}
