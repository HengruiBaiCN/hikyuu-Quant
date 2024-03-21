/*
 * AllocateMoney.h
 *
 *  Created on: 2018年1月30日
 *      Author: fasiondog
 */

#pragma once
#ifndef TRADE_SYS_ALLOCATEFUNDS_ALLOCATEFUNDSBASE_H_
#define TRADE_SYS_ALLOCATEFUNDS_ALLOCATEFUNDSBASE_H_

#include "../../utilities/Parameter.h"
#include "../selector/SystemWeight.h"

namespace hku {

/**
 * 资产分配调整算法
 * @details 根据资产市值对资产比例进行分配调整。单纯的资金调整，请使用资金管理算法。
 * @ingroup AllocateFunds
 */
class HKU_API AllocateFundsBase : public enable_shared_from_this<AllocateFundsBase> {
    PARAMETER_SUPPORT

public:
    /** 默认构造函数 */
    AllocateFundsBase();

    /**
     * 构造函数
     * @param name 算法名称
     */
    AllocateFundsBase(const string& name);

    /** 析构函数 */
    virtual ~AllocateFundsBase();

    /** 获取算法名称 */
    const string& name() const;

    /** 修改算法名称 */
    void name(const string& name);

    /**
     * 执行资产分配调整，仅供 PF 调用
     * @param date 指定日期
     * @param se_list 系统实例选择器选出的系统实例
     * @param running_list 当前运行中的系统实例
     * @param ignore_list 忽略不进行调仓的运行中系统
     * @return 需延迟执行卖出操作的系统列表，其中权重为相应需卖出的数量
     */
    SystemWeightList adjustFunds(const Datetime& date, const SystemWeightList& se_list,
                                 const std::unordered_set<SYSPtr>& running_list);

    /** 获取交易账户 */
    const TMPtr& getTM() const;

    /** 设定交易账户，由 PF 设定 */
    void setTM(const TMPtr&);

    /** 设置 Portfolio 的影子账户, 仅由 Portfolio 调用 */
    void setShadowTM(const TMPtr&);

    const TMPtr& getShadowTM(const TMPtr&) const;

    /** 获取关联查询条件 */
    const KQuery& getQuery() const;

    /** 设置查询条件， 由 PF 设定 */
    void setQuery(const KQuery& query);

    /** 获取当前不参与资产分配的保留比例 */
    double getReservePercent();

    /**
     * 设置不参与资产分配的保留比例，该比例在执行reset时会被置为参数 default_reserve_percent 的值
     * @note 主要用分配算法动态控制不参与分配的资产比例
     * @param p 取值范围[0,1]，小于0将被强制置为0， 大于1将被置为1
     */
    void setReservePercent(double p);

    /** 复位 */
    void reset();

    typedef shared_ptr<AllocateFundsBase> AFPtr;

    /** 克隆操作 */
    AFPtr clone();

    /** 子类复位接口 */
    virtual void _reset() {}

    /** 子类克隆私有变量接口 */
    virtual AFPtr _clone() = 0;

    /**
     * 子类分配权重接口，获取实际分配资产的系统实例及其权重
     * @details 实际调用子类接口 _allocateWeight
     * @param date 指定日期
     * @param se_list 系统实例选择器选出的系统实例
     * @return 子类只需要返回每个系统的相对比例即可
     */
    virtual SystemWeightList _allocateWeight(const Datetime& date,
                                             const SystemWeightList& se_list) = 0;

private:
    /* 同时调整已运行中的子系统（已分配资金或已持仓） */
    SystemWeightList _adjust_with_running(const Datetime& date, const SystemWeightList& se_list,
                                          const std::unordered_set<SYSPtr>& running_list);

    /* 不调整已在运行中的子系统 */
    void _adjust_without_running(const Datetime& date, const SystemWeightList& se_list,
                                 const std::unordered_set<SYSPtr>& running_list);

    /* 检查分配的权重是否在 0 和 1 之间，如果存在错误，抛出异常，仅在 trace 时生效*/
    void _check_weight(const SystemWeightList&);

private:
    string m_name;      // 组件名称
    KQuery m_query;     // 查询条件
    TMPtr m_tm;         // 运行期由PF设定，PF的实际账户
    TMPtr m_shadow_tm;  // 运行期由PF设定，tm 的影子账户，由于协调分配资金
    double m_reserve_percent;  // 保留资产比例，不参与资产分配

//============================================
// 序列化支持
//============================================
#if HKU_SUPPORT_SERIALIZATION
private:
    friend class boost::serialization::access;
    template <class Archive>
    void save(Archive& ar, const unsigned int version) const {
        ar& BOOST_SERIALIZATION_NVP(m_name);
        ar& BOOST_SERIALIZATION_NVP(m_params);
        ar& BOOST_SERIALIZATION_NVP(m_query);
        ar& BOOST_SERIALIZATION_NVP(m_reserve_percent);
    }

    template <class Archive>
    void load(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_NVP(m_name);
        ar& BOOST_SERIALIZATION_NVP(m_params);
        ar& BOOST_SERIALIZATION_NVP(m_query);
        ar& BOOST_SERIALIZATION_NVP(m_reserve_percent);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif /* HKU_SUPPORT_SERIALIZATION */
};

#if HKU_SUPPORT_SERIALIZATION
BOOST_SERIALIZATION_ASSUME_ABSTRACT(SelectorBase)
#endif

#if HKU_SUPPORT_SERIALIZATION
/**
 * 对于没有私有变量的继承子类，可直接使用该宏定义序列化
 * @code
 * class Drived: public AllocateFundsBase {
 *     ALLOCATEFUNDS_NO_PRIVATE_MEMBER_SERIALIZATION
 *
 * public:
 *     Drived();
 *     ...
 * };
 * @endcode
 * @ingroup Selector
 */
#define ALLOCATEFUNDS_NO_PRIVATE_MEMBER_SERIALIZATION               \
private:                                                            \
    friend class boost::serialization::access;                      \
    template <class Archive>                                        \
    void serialize(Archive& ar, const unsigned int version) {       \
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(AllocateFundsBase); \
    }
#else
#define ALLOCATEFUNDS_NO_PRIVATE_MEMBER_SERIALIZATION
#endif

#define ALLOCATEFUNDS_IMP(classname)   \
public:                                \
    virtual AFPtr _clone() override {  \
        return AFPtr(new classname()); \
    }                                  \
    virtual SystemWeightList _allocateWeight(const Datetime&, const SystemWeightList&) override;

typedef shared_ptr<AllocateFundsBase> AllocateFundsPtr;
typedef shared_ptr<AllocateFundsBase> AFPtr;

HKU_API std::ostream& operator<<(std::ostream&, const AllocateFundsBase&);
HKU_API std::ostream& operator<<(std::ostream&, const AFPtr&);

inline const string& AllocateFundsBase::name() const {
    return m_name;
}

inline void AllocateFundsBase::name(const string& name) {
    m_name = name;
}

inline const TMPtr& AllocateFundsBase::getTM() const {
    return m_tm;
}

inline void AllocateFundsBase::setTM(const TMPtr& tm) {
    m_tm = tm;
}

inline void AllocateFundsBase::setShadowTM(const TMPtr& tm) {
    m_shadow_tm = tm;
}

inline const TMPtr& AllocateFundsBase::getShadowTM(const TMPtr&) const {
    return m_shadow_tm;
}

inline const KQuery& AllocateFundsBase::getQuery() const {
    return m_query;
}

inline void AllocateFundsBase::setQuery(const KQuery& query) {
    m_query = query;
}

inline double AllocateFundsBase::getReservePercent() {
    return m_reserve_percent;
}

} /* namespace hku */

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hku::AllocateFundsBase> : ostream_formatter {};

template <>
struct fmt::formatter<hku::AFPtr> : ostream_formatter {};
#endif

#endif /* TRADE_SYS_ALLOCATEFUNDS_ALLOCATEFUNDSBASE_H_ */
