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

#ifndef HASHKAT_ACTION_ST_HPP_
#define HASHKAT_ACTION_ST_HPP_

namespace hashkat {

////////////////////////////////////////////////////////////////////////////////
// Abstract base class for actions

template
<
    class NetworkType
,   class ContentsType
,   class ConfigType
,   class RngType
,   class TimeType
>
class action_base
:   private boost::noncopyable
{
public:
    typedef typename NetworkType::rate_type rate_type;
    typedef typename NetworkType::rate_type weight_type;
    typedef boost::signals2::signal<void()> action_happened_signal_type;
    typedef boost::signals2::signal<void()> action_finished_signal_type;

    action_base()
    :   rate_(0)
    ,   weight_(0)
    {}

    void init(
        NetworkType& net
    ,   ContentsType& cnt
    ,   ConfigType& cnf
    ,   RngType& rng
    ,   const TimeType& time)
    {   do_init(net, cnt, cnf, rng, time);   }

    rate_type rate() const
    {   return rate_;   }

    weight_type weight() const
    {   return weight_;   }

    void post_init()
    {   do_post_init(); }

    void reset()
    {   do_reset(); }

    void update_weight()
    {   do_update_weight();   }

    void operator()()
    {   do_action();   }

    std::ostream& print(std::ostream& out) const
    {   return do_print(out); }

    void dump(const std::string& folder) const
    {   return do_dump(folder);   }

    action_happened_signal_type& happened()
    {   return action_happened_signal_;   }

    action_finished_signal_type& finished()
    {   return action_finished_signal_;   }


// Implementation
    virtual ~action_base() {};

protected:
    std::size_t rate_;
    weight_type weight_;
    action_happened_signal_type action_happened_signal_;
    action_happened_signal_type action_finished_signal_;

private:
    virtual void do_init(
        NetworkType& net
    ,   ContentsType& cnt
    ,   ConfigType& cnf
    ,   RngType& rng
    ,   const TimeType& time) = 0;
    virtual void do_post_init() = 0;
    virtual void do_reset() = 0;
    virtual void do_update_weight() = 0;
    virtual void do_action() = 0;
    virtual std::ostream& do_print(std::ostream& out) const = 0;
    virtual void do_dump(const std::string& folder) const = 0;
};

template
<
    class NetworkType
,   class ContentsType
,   class ConfigType
,   class RngType
,   class TimeType
>
std::ostream& operator<< (
    std::ostream& out
,   const action_base
        <NetworkType, ContentsType, ConfigType, RngType, TimeType>* ptr)
{
    return ptr->print(out);
}

}    // namespace hashkat

#endif  // HASHKAT_ACTION_ST_HPP_
