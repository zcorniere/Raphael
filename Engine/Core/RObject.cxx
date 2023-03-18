#include "Engine/Core/RObject.hxx"

#include <unordered_set>

static std::unordered_set<void *> s_LiveReferences;
static std::recursive_mutex s_LiveReferenceMutex;

namespace RObjectUtils
{

void AddToLiveReferences(void *instance)
{
    check(instance);

    std::scoped_lock lock(s_LiveReferenceMutex);
    s_LiveReferences.insert(instance);
}

void RemoveFromLiveReferences(void *instance)
{
    check(instance);

    std::scoped_lock lock(s_LiveReferenceMutex);
    check(s_LiveReferences.find(instance) != s_LiveReferences.end());
    s_LiveReferences.erase(instance);
}

bool IsLive(void *instance)
{
    check(instance);
    std::scoped_lock lock(s_LiveReferenceMutex);
    return s_LiveReferences.find(instance) != s_LiveReferences.end();
}

bool AreThereAnyLiveObject(bool bPrintObjects)
{
    std::scoped_lock lock(s_LiveReferenceMutex);

    if (bPrintObjects && s_LiveReferences.size() > 0) {
        for (void *ObjectPtr: s_LiveReferences) {
            Ref<RObject> Object((RObject *)ObjectPtr);
            LOG(LogRObject, Debug, "RObject<{}> ({:p}) have {} references", Object->GetName(), ObjectPtr,
                Object->GetRefCount());
        }
    }
    return s_LiveReferences.size() > 0;
}

}    // namespace RObjectUtils

void RObject::AddParent(RObject *InParent)
{
    AddParent(Ref<RObject>(InParent));
}
