/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

//
// SplitStatistics is intended for deployments where statistics information is
// collected both split over various disjoint domains (e.g. vhosts) and
// globally, with the class making sure to update both the local and global
// fragments appropriately. Also included are its variable, timed variable,
// and histogram implementations.

#ifndef PAGESPEED_KERNEL_BASE_SPLIT_STATISTICS_H_
#define PAGESPEED_KERNEL_BASE_SPLIT_STATISTICS_H_

#include "pagespeed/kernel/base/abstract_mutex.h"
#include "pagespeed/kernel/base/basictypes.h"
#include "pagespeed/kernel/base/scoped_ptr.h"
#include "pagespeed/kernel/base/statistics.h"
#include "pagespeed/kernel/base/statistics_template.h"
#include "pagespeed/kernel/base/string_util.h"  // for StringPiece
#include "pagespeed/kernel/base/writer.h"

namespace net_instaweb {

class MessageHandler;
class StatisticsLogger;
class ThreadSystem;

// A statistics variable that forwards writes to two other Variable objects,
// but reads only from one.
class SplitUpDownCounter : public UpDownCounter {
 public:
  // UpDownCounter 'rw' will be used to read and write, variable 'w'
  // will be used for writes only. Does not take ownership of either
  // 'rw' or 'w'. 'rw' and 'w' must be non-NULL.
  SplitUpDownCounter(UpDownCounter* rw, UpDownCounter* w);
  ~SplitUpDownCounter() override;
  void Set(int64 new_value) override;
  int64 SetReturningPreviousValue(int64 new_value) override;
  int64 Get() const override;
  StringPiece GetName() const override;
  int64 AddHelper(int64 delta) override;

 private:
  UpDownCounter* rw_;
  UpDownCounter* w_;
  DISALLOW_COPY_AND_ASSIGN(SplitUpDownCounter);
};

class SplitVariable : public Variable {
 public:
  // Variable 'rw' will be used to read and write, variable 'w'
  // will be used for writes only. Does not take ownership of either
  // 'rw' or 'w'. 'rw' and 'w' must be non-NULL.
  SplitVariable(Variable* rw, Variable* w);
  ~SplitVariable() override;
  int64 Get() const override;
  StringPiece GetName() const override;
  int64 AddHelper(int64 delta) override;
  void Clear() override;

 private:
  Variable* rw_;
  Variable* w_;
  DISALLOW_COPY_AND_ASSIGN(SplitVariable);
};

// A histogram that forwards writes to two other Histogram objects,
// but reads only from one.
class SplitHistogram : public Histogram {
 public:
  // Histogram 'rw' will be used to read and write, histogram 'w'
  // will be used for writes only. Does not take ownership of either
  // 'rw' or 'w'. 'rw' and 'w' must be non-NULL.
  SplitHistogram(ThreadSystem* thread_system, Histogram* rw, Histogram* w);
  ~SplitHistogram() override;

  // Reimplementation of the histogram API. See the base class for method
  // descriptions.
  void Add(double value) override;
  void Clear() override;
  void Render(int index, Writer* writer, MessageHandler* handler) override;
  int NumBuckets() override;
  void EnableNegativeBuckets() override;
  void SetMinValue(double value) override;
  void SetMaxValue(double value) override;
  void SetSuggestedNumBuckets(int i) override;
  double BucketStart(int index) override;
  double BucketLimit(int index) override;
  double BucketCount(int index) override;

 protected:
  double AverageInternal() override;
  double PercentileInternal(const double perc) override;
  double StandardDeviationInternal() override;
  double CountInternal() override;
  double MaximumInternal() override;
  double MinimumInternal() override;

  AbstractMutex* lock() override;

 private:
  std::unique_ptr<AbstractMutex> lock_;
  Histogram* rw_;
  Histogram* w_;

  DISALLOW_COPY_AND_ASSIGN(SplitHistogram);
};

// A timed variable that forwards writes writes to two other TimedVariable
// objects, but reads only from one.
class SplitTimedVariable : public TimedVariable {
 public:
  // TimedVariable 'rw' will be used to read and write, histogram 'w'
  // will be used for writes only. Does not take ownership of either
  // 'rw' or 'w'. 'rw' and 'w' must be non-NULL.
  SplitTimedVariable(TimedVariable* rw, TimedVariable* w);
  ~SplitTimedVariable() override;

  void IncBy(int64 delta) override;
  int64 Get(int level) override;
  void Clear() override;

 private:
  TimedVariable* rw_;
  TimedVariable* w_;

  DISALLOW_COPY_AND_ASSIGN(SplitTimedVariable);
};

class SplitStatistics
    : public StatisticsTemplate<SplitVariable, SplitUpDownCounter,
                                SplitHistogram, SplitTimedVariable> {
 public:
  // Initializes a statistics splitter which proxies 'local' but also forwards
  // writes to 'global' for aggregation with other SplitStatistics instances.
  // Takes ownership of 'local', but not thread_system or global.
  //
  // Note that before AddUpDownCounter or similar methods are invoked on
  // this object (which is usually done by static
  // ::InitStats(Statistics* methods) they must have been invoked on
  // both local and global statistics objects for the same object
  // names.
  SplitStatistics(ThreadSystem* thread_system, Statistics* local,
                  Statistics* global);

  ~SplitStatistics() override;

  StatisticsLogger* console_logger() override {
    // console_logger() is only used for read access, so just provide the
    // local version.
    return local_->console_logger();
  }

 protected:
  SplitUpDownCounter* NewUpDownCounter(StringPiece name) override;
  SplitVariable* NewVariable(StringPiece name) override;
  SplitUpDownCounter* NewGlobalUpDownCounter(StringPiece name) override;
  SplitHistogram* NewHistogram(StringPiece name) override;
  SplitTimedVariable* NewTimedVariable(StringPiece name) override;

 private:
  ThreadSystem* thread_system_;
  std::unique_ptr<Statistics> local_;
  Statistics* global_;
  DISALLOW_COPY_AND_ASSIGN(SplitStatistics);
};

}  // namespace net_instaweb

#endif  // PAGESPEED_KERNEL_BASE_SPLIT_STATISTICS_H_
