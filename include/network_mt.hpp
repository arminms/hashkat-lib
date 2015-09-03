////////////////////////////////////////////////////////////////////////////////
// This file is part of the #KAT Social Network Simulator.
//
// The #KAT Social Network Simulator is free software: you can redistribute it
// and/or modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation, either version 3 of the License,
// or (at your option) any later version.
//
// The #KAT Social Network Simulator is distributed in the hope that it will be
// useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
// Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// the #KAT Social Network Simulator. If not, see http://www.gnu.org/licenses.
//
// Addendum:
//
// Under this license, derivations of the #KAT Social Network Simulator
// typically must be provided in source form. The #KAT Social Network Simulator
// and derivations thereof may be relicensed by decision of the original
// authors (Kevin Ryczko & Adam Domurad, Isaac Tamblyn), as well, in the case
// of a derivation, subsequent authors.
//

#ifndef HASHKAT_NETWORK_MT_H_
#define HASHKAT_NETWORK_MT_H_

#include <mutex>
#include <atomic>
#include <tbb/concurrent_vector.h>
#include <tbb/concurrent_unordered_set.h>

namespace hashkat {

template
<
    class    AgentType
,   class    ConfigType
,   typename T = std::uint32_t
,   typename ValueType = double
>
class network_mt
{
public:
    typedef T type;
    typedef ValueType rate_type;
    typedef ValueType value_type;
    typedef AgentType agent_type;
    typedef ConfigType config_type;
    typedef network_mt<AgentType, ConfigType, T, ValueType> self_type;
    typedef boost::signals2::signal<void(T)> grown_signal_type;
    typedef boost::signals2::signal<void(T, T)> connection_added_signal_type;
    typedef boost::signals2::signal<void(T, T)> connection_removed_signal_type;

    network_mt()
    :   agents_(nullptr)
    ,   n_agents_(0)
    ,   max_agents_(0)
    {}

    network_mt(const ConfigType& conf)
    :   agents_(nullptr)
    ,   n_agents_(0)
    ,   max_agents_(0)
    {   allocate(conf.template get<T>("hashkat.network.max_agents", 1000)); }

    network_mt(T n)
    :   agents_(nullptr)
    ,   n_agents_(0)
    ,   max_agents_(0)
    {   allocate(n); }

    ~network_mt()
    {   delete[] agents_;   }

    void reset()
    {
        n_agents_ = 0;
        followers_.clear();
        followees_.clear();
    }

    void allocate(T n)
    {
        if (agents_)
            delete[] agents_;
        max_agents_ = n;
        agents_ = new AgentType[max_agents_];
        followers_.reserve(max_agents_);
        followees_.reserve(max_agents_);
    }

    bool grow()
    {
        if (n_agents_ < max_agents_)
        {
            {
                std::lock_guard<std::mutex> lg(grow_mutex_);
                followers_.emplace_back(tbb::concurrent_unordered_set<T>());
                followees_.emplace_back(tbb::concurrent_unordered_set<T>());
            }
            ++n_agents_;
            grown_signal_(n_agents_ - 1);
            return true;
        }
        else 
            return false;
    }

    T grow(T n)
    {
        for (auto i = 0; i < n; ++i)
        {
            if (n_agents_ == max_agents_)
                return i;
            followers_.emplace_back(tbb::concurrent_unordered_set<T>());
            followees_.emplace_back(tbb::concurrent_unordered_set<T>());
            ++n_agents_;
            grown_signal_(n_agents_ - 1);
        }
        return n;
    }

    T size() const
    {   return n_agents_;   }

    T max_size() const
    {   return max_agents_; }

    AgentType& back()
    {   return (*this)[n_agents_ - 1];  }

    AgentType* begin()
    {   return agents_; }

    AgentType* end()
    {   return agents_ + n_agents_; }

    AgentType& operator[](T idx)
    {
        BOOST_ASSERT_MSG(idx >= 0 && idx < n_agents_,
            "network out-of-bounds agent access :(");
        return agents_[idx];
    }

    grown_signal_type& grown()
    {   return grown_signal_;   }

    connection_added_signal_type& connection_added()
    {   return connection_added_signal_;    }

    connection_removed_signal_type& connection_removed()
    {   return connection_removed_signal_;  }

    T followees_size(T id) const
    {
        return followees_[id].size();
    }

    T followers_size(T id) const
    {
        return followers_[id].size();
    }

    bool can_grow() const
    {
        return n_agents_ < max_agents_;
    }

    bool have_connection(T followee_id, T follower_id) const
    {
        return (    followers_[followee_id].find(follower_id)
               !=   followers_[followee_id].end()   );
    }

    bool connect(T followee_id, T follower_id)
    {
        BOOST_ASSERT_MSG(followee_id != follower_id,
            "agent cannot be connected to itself :(");

        if (followers_[followee_id].insert(follower_id).second
        &&  followees_[follower_id].insert(followee_id).second)
        {
            connection_added_signal_(followee_id, follower_id);
            return true;
        }
        else
            return false;
    }

    bool disconnect(T unfollowee_id, T unfollower_id)
    {
        BOOST_ASSERT_MSG(unfollowee_id != unfollower_id,
            "agent cannot be disconnected from itself :(");

        bool r = false;
        {
            std::lock_guard<std::mutex> lg(erase_mutex_);
            r = followers_[unfollowee_id].unsafe_erase(unfollower_id);
            followees_[unfollower_id].unsafe_erase(unfollowee_id);
        }
        if (r)
            connection_removed_signal_(unfollowee_id, unfollower_id);
        return r;
    }

    std::ostream& print(std::ostream& out) const
    {
        out << "# Maximum Number of Agents: " << max_agents_ << std::endl;
        out << "# Number of Agents: " << n_agents_ << std::endl;
        out << "# Network: " << std::endl;

        for (auto i = 0; i < n_agents_; ++i)
        {
            out << std::setfill('0') << std::setw(7) << i << std::endl;

            out << "    +---[in]" << std::endl
                << "        \\--- "
                << followers_[i].size()
                << " -> ";
            for (auto follower : followers_[i])
                out << follower << ',';
            out << std::endl;

            out << "    +---[out]" << std::endl
                << "        \\--- "
                << followees_[i].size()
                << " -> ";
            for (auto following : followees_[i])
                out << following << ',';
            out << std::endl;
        }
        return out;
    }

private:
    // member variables
    AgentType* agents_;
    std::atomic<T> n_agents_;
    T max_agents_;
    //std::vector<tbb::concurrent_unordered_set<T>> followers_;
    //std::vector<tbb::concurrent_unordered_set<T>> followees_;
    tbb::concurrent_vector<tbb::concurrent_unordered_set<T>> followers_;
    tbb::concurrent_vector<tbb::concurrent_unordered_set<T>> followees_;
    std::mutex grow_mutex_;
    std::mutex erase_mutex_;
    grown_signal_type grown_signal_;
    connection_added_signal_type connection_added_signal_;
    connection_removed_signal_type connection_removed_signal_;
};

template
<
    class    AgentType
,   class    ConfigType
,   typename T
,   typename ValueType
>
std::ostream& operator<< (
    std::ostream& out
,   const network_mt<AgentType, ConfigType, T, ValueType>& n)
{
    return n.print(out);
}

}    // namespace hashkat

#endif  // HASHKAT_NETWORK_MT_H_