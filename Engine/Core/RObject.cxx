
#include <unordered_set>

namespace Raphael
{

static std::unordered_set<void *> s_LiveReferences;
static std::mutex s_LiveReferenceMutex;

namespace RObjectUtils
{

    void AddToLiveReferences(void *instance)
    {
        std::scoped_lock<std::mutex> lock(s_LiveReferenceMutex);
        check(instance);
        s_LiveReferences.insert(instance);
    }

    void RemoveFromLiveReferences(void *instance)
    {
        std::scoped_lock<std::mutex> lock(s_LiveReferenceMutex);
        check(instance);
        check(s_LiveReferences.find(instance) != s_LiveReferences.end());
        s_LiveReferences.erase(instance);
    }

    bool IsLive(void *instance)
    {
        check(instance);
        return s_LiveReferences.find(instance) != s_LiveReferences.end();
    }
}    // namespace RObjectUtils

}    // namespace Raphael
