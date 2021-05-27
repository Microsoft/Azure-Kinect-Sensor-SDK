//------------------------------------------------------------------------------
// <copyright file="ILoggingProvider.cs" company="Microsoft">
// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
// </copyright>
//------------------------------------------------------------------------------
using System;
using System.Collections.Generic;
using System.Text;

namespace Microsoft.Azure.Kinect.Sensor
{
    /// <summary>
    /// An interface for trace logging providers from the native SDK.
    /// </summary>
    public interface ILoggingProvider
    {
#pragma warning disable CA1003 // Use generic event handler instances
        /// <summary>
        /// Occurs when the native SDK delivers a debug message.
        /// </summary>
        event Action<LogMessage> LogMessage;
#pragma warning restore CA1003 // Use generic event handler instances

        /// <summary>
        /// Gets the name of the layer providing the messages.
        /// </summary>
        string ProviderName { get; }
    }
}
