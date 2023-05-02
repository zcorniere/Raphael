#include "Engine/Core/RObject.hxx"

#include <unordered_set>

static std::unordered_set<RObject *> s_LiveReferences;
static std::recursive_mutex s_LiveReferenceMutex;

namespace RObjectUtils
{

void AddToLiveReferences(RObject *instance)
{
    check(instance);

    std::scoped_lock lock(s_LiveReferenceMutex);
    s_LiveReferences.insert(instance);
}

void RemoveFromLiveReferences(RObject *instance)
{
    check(instance);

    std::scoped_lock lock(s_LiveReferenceMutex);
    check(s_LiveReferences.find(instance) != s_LiveReferences.end());
    s_LiveReferences.erase(instance);
}

bool IsLive(RObject *instance)
{
    check(instance);
    std::scoped_lock lock(s_LiveReferenceMutex);
    return s_LiveReferences.find(instance) != s_LiveReferences.end();
}

bool AreThereAnyLiveObject(bool bPrintObjects)
{
    std::scoped_lock lock(s_LiveReferenceMutex);

    if (bPrintObjects) {
        for (RObject *ObjectPtr: s_LiveReferences) {
            LOG(LogRObject, Trace, "RObject<{}> ({:p}) have {} references", ObjectPtr->GetName(), (void *)ObjectPtr,
                ObjectPtr->GetRefCount());
        }
    }
    return s_LiveReferences.size() > 0;
}

}    // namespace RObjectUtils
