// Code generated by memo_table_types.gen.go.tmpl. DO NOT EDIT.

// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

package encoding

import (
	"github.com/apache/arrow/go/v6/arrow/memory"
	"github.com/apache/arrow/go/v6/parquet"
)

// standard map based implementation of memo tables which can be more efficient
// in some cases based on the uniqueness / amount / size of the data.
// these are left here for now for use in the benchmarks to compare against the
// custom hash table implementation in the internal/hashing package as a base
// benchmark comparison.

func NewInt32MemoTable(memory.Allocator) MemoTable {
	return &int32MemoTableImpl{
		table: make(map[int32]struct {
			value     int32
			memoIndex int
		}),
		nullIndex: keyNotFound,
	}
}

type int32MemoTableImpl struct {
	table map[int32]struct {
		value     int32
		memoIndex int
	}
	nullIndex int
}

func (m *int32MemoTableImpl) Reset() {
	m.table = make(map[int32]struct {
		value     int32
		memoIndex int
	})
	m.nullIndex = keyNotFound
}

func (m *int32MemoTableImpl) GetNull() (int, bool) {
	return m.nullIndex, m.nullIndex != keyNotFound
}

func (m *int32MemoTableImpl) Size() int {
	sz := len(m.table)
	if _, ok := m.GetNull(); ok {
		sz++
	}
	return sz
}

func (m *int32MemoTableImpl) GetOrInsertNull() (idx int, found bool) {
	idx, found = m.GetNull()
	if !found {
		idx = m.Size()
		m.nullIndex = idx
	}
	return
}

func (m *int32MemoTableImpl) Get(val interface{}) (int, bool) {
	v := val.(int32)
	if p, ok := m.table[v]; ok {
		return p.memoIndex, true
	}
	return keyNotFound, false
}

func (m *int32MemoTableImpl) GetOrInsert(val interface{}) (idx int, found bool, err error) {
	v := val.(int32)
	p, ok := m.table[v]
	if ok {
		idx = p.memoIndex
	} else {
		idx = m.Size()
		p.value = v
		p.memoIndex = idx
		m.table[v] = p
		found = true
	}
	return
}

func (m *int32MemoTableImpl) WriteOut(out []byte) {
	m.CopyValuesSubset(0, out)
}

func (m *int32MemoTableImpl) WriteOutSubset(start int, out []byte) {
	m.CopyValuesSubset(start, out)
}

func (m *int32MemoTableImpl) CopyValues(out interface{}) {
	m.CopyValuesSubset(0, out)
}

func (m *int32MemoTableImpl) CopyValuesSubset(start int, out interface{}) {
	outval := out.([]int32)
	for _, v := range m.table {
		idx := v.memoIndex - start
		if idx >= 0 {
			outval[idx] = v.value
		}
	}
}

func NewInt64MemoTable(memory.Allocator) MemoTable {
	return &int64MemoTableImpl{
		table: make(map[int64]struct {
			value     int64
			memoIndex int
		}),
		nullIndex: keyNotFound,
	}
}

type int64MemoTableImpl struct {
	table map[int64]struct {
		value     int64
		memoIndex int
	}
	nullIndex int
}

func (m *int64MemoTableImpl) Reset() {
	m.table = make(map[int64]struct {
		value     int64
		memoIndex int
	})
	m.nullIndex = keyNotFound
}

func (m *int64MemoTableImpl) GetNull() (int, bool) {
	return m.nullIndex, m.nullIndex != keyNotFound
}

func (m *int64MemoTableImpl) Size() int {
	sz := len(m.table)
	if _, ok := m.GetNull(); ok {
		sz++
	}
	return sz
}

func (m *int64MemoTableImpl) GetOrInsertNull() (idx int, found bool) {
	idx, found = m.GetNull()
	if !found {
		idx = m.Size()
		m.nullIndex = idx
	}
	return
}

func (m *int64MemoTableImpl) Get(val interface{}) (int, bool) {
	v := val.(int64)
	if p, ok := m.table[v]; ok {
		return p.memoIndex, true
	}
	return keyNotFound, false
}

func (m *int64MemoTableImpl) GetOrInsert(val interface{}) (idx int, found bool, err error) {
	v := val.(int64)
	p, ok := m.table[v]
	if ok {
		idx = p.memoIndex
	} else {
		idx = m.Size()
		p.value = v
		p.memoIndex = idx
		m.table[v] = p
		found = true
	}
	return
}

func (m *int64MemoTableImpl) WriteOut(out []byte) {
	m.CopyValuesSubset(0, out)
}

func (m *int64MemoTableImpl) WriteOutSubset(start int, out []byte) {
	m.CopyValuesSubset(start, out)
}

func (m *int64MemoTableImpl) CopyValues(out interface{}) {
	m.CopyValuesSubset(0, out)
}

func (m *int64MemoTableImpl) CopyValuesSubset(start int, out interface{}) {
	outval := out.([]int64)
	for _, v := range m.table {
		idx := v.memoIndex - start
		if idx >= 0 {
			outval[idx] = v.value
		}
	}
}

func NewInt96MemoTable(memory.Allocator) MemoTable {
	return &int96MemoTableImpl{
		table: make(map[parquet.Int96]struct {
			value     parquet.Int96
			memoIndex int
		}),
		nullIndex: keyNotFound,
	}
}

type int96MemoTableImpl struct {
	table map[parquet.Int96]struct {
		value     parquet.Int96
		memoIndex int
	}
	nullIndex int
}

func (m *int96MemoTableImpl) Reset() {
	m.table = make(map[parquet.Int96]struct {
		value     parquet.Int96
		memoIndex int
	})
	m.nullIndex = keyNotFound
}

func (m *int96MemoTableImpl) GetNull() (int, bool) {
	return m.nullIndex, m.nullIndex != keyNotFound
}

func (m *int96MemoTableImpl) Size() int {
	sz := len(m.table)
	if _, ok := m.GetNull(); ok {
		sz++
	}
	return sz
}

func (m *int96MemoTableImpl) GetOrInsertNull() (idx int, found bool) {
	idx, found = m.GetNull()
	if !found {
		idx = m.Size()
		m.nullIndex = idx
	}
	return
}

func (m *int96MemoTableImpl) Get(val interface{}) (int, bool) {
	v := val.(parquet.Int96)
	if p, ok := m.table[v]; ok {
		return p.memoIndex, true
	}
	return keyNotFound, false
}

func (m *int96MemoTableImpl) GetOrInsert(val interface{}) (idx int, found bool, err error) {
	v := val.(parquet.Int96)
	p, ok := m.table[v]
	if ok {
		idx = p.memoIndex
	} else {
		idx = m.Size()
		p.value = v
		p.memoIndex = idx
		m.table[v] = p
		found = true
	}
	return
}

func (m *int96MemoTableImpl) WriteOut(out []byte) {
	m.CopyValuesSubset(0, out)
}

func (m *int96MemoTableImpl) WriteOutSubset(start int, out []byte) {
	m.CopyValuesSubset(start, out)
}

func (m *int96MemoTableImpl) CopyValues(out interface{}) {
	m.CopyValuesSubset(0, out)
}

func (m *int96MemoTableImpl) CopyValuesSubset(start int, out interface{}) {
	outval := out.([]parquet.Int96)
	for _, v := range m.table {
		idx := v.memoIndex - start
		if idx >= 0 {
			outval[idx] = v.value
		}
	}
}

func NewFloat32MemoTable(memory.Allocator) MemoTable {
	return &float32MemoTableImpl{
		table: make(map[float32]struct {
			value     float32
			memoIndex int
		}),
		nullIndex: keyNotFound,
	}
}

type float32MemoTableImpl struct {
	table map[float32]struct {
		value     float32
		memoIndex int
	}
	nullIndex int
}

func (m *float32MemoTableImpl) Reset() {
	m.table = make(map[float32]struct {
		value     float32
		memoIndex int
	})
	m.nullIndex = keyNotFound
}

func (m *float32MemoTableImpl) GetNull() (int, bool) {
	return m.nullIndex, m.nullIndex != keyNotFound
}

func (m *float32MemoTableImpl) Size() int {
	sz := len(m.table)
	if _, ok := m.GetNull(); ok {
		sz++
	}
	return sz
}

func (m *float32MemoTableImpl) GetOrInsertNull() (idx int, found bool) {
	idx, found = m.GetNull()
	if !found {
		idx = m.Size()
		m.nullIndex = idx
	}
	return
}

func (m *float32MemoTableImpl) Get(val interface{}) (int, bool) {
	v := val.(float32)
	if p, ok := m.table[v]; ok {
		return p.memoIndex, true
	}
	return keyNotFound, false
}

func (m *float32MemoTableImpl) GetOrInsert(val interface{}) (idx int, found bool, err error) {
	v := val.(float32)
	p, ok := m.table[v]
	if ok {
		idx = p.memoIndex
	} else {
		idx = m.Size()
		p.value = v
		p.memoIndex = idx
		m.table[v] = p
		found = true
	}
	return
}

func (m *float32MemoTableImpl) WriteOut(out []byte) {
	m.CopyValuesSubset(0, out)
}

func (m *float32MemoTableImpl) WriteOutSubset(start int, out []byte) {
	m.CopyValuesSubset(start, out)
}

func (m *float32MemoTableImpl) CopyValues(out interface{}) {
	m.CopyValuesSubset(0, out)
}

func (m *float32MemoTableImpl) CopyValuesSubset(start int, out interface{}) {
	outval := out.([]float32)
	for _, v := range m.table {
		idx := v.memoIndex - start
		if idx >= 0 {
			outval[idx] = v.value
		}
	}
}
