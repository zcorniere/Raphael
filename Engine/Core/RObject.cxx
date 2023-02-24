
#include <unordered_set>

static std::unordered_set<void *> s_LiveReferences;
static std::mutex s_LiveReferenceMutex;

namespace RObjectUtils
{

void AddToLiveReferences(void *instance)
{
    check(instance);

    std::scoped_lock<std::mutex> lock(s_LiveReferenceMutex);
    s_LiveReferences.insert(instance);
}

void RemoveFromLiveReferences(void *instance)
{
    check(instance);

    std::scoped_lock<std::mutex> lock(s_LiveReferenceMutex);
    check(s_LiveReferences.find(instance) != s_LiveReferences.end());
    s_LiveReferences.erase(instance);
}

bool IsLive(void *instance)
{
    check(instance);
    std::scoped_lock<std::mutex> lock(s_LiveReferenceMutex);
    return s_LiveReferences.find(instance) != s_LiveReferences.end();
}
}    // namespace RObjectUtils
